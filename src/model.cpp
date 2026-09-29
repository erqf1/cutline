#include "model.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>

QString fmtTime(double t) {
    t = std::max(0.0, t);
    int m = int(t / 60);
    double s = t - m * 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 4, 'f', 1, QChar('0'));
}

QString ffmpegPath() {
#ifdef Q_OS_WIN
    const QString name = "ffmpeg.exe";
#else
    const QString name = "ffmpeg";
#endif
    const QString dir = QCoreApplication::applicationDirPath();
    for (const QString& c : {dir + "/" + name, dir + "/../lib/cutline/" + name, dir + "/../Resources/" + name})
        if (QFileInfo::exists(c)) return QFileInfo(c).absoluteFilePath();
#ifdef Q_OS_MAC
    for (const char* c : {"/opt/homebrew/bin/ffmpeg", "/usr/local/bin/ffmpeg"})
        if (QFileInfo::exists(c)) return c;
#endif
    return "ffmpeg";
}

QString detectEncoder() {
    static QString cached;
    if (!cached.isEmpty()) return cached;
    for (const char* enc : {"h264_nvenc", "h264_qsv", "h264_amf", "h264_videotoolbox"}) {
        QProcess p;
        p.start(ffmpegPath(), {"-v", "error", "-f", "lavfi", "-i", "color=c=black:s=256x256:d=0.2",
                               "-c:v", enc, "-f", "null", "-"});
        if (p.waitForFinished(15000) && p.exitCode() == 0) return cached = enc;
    }
    return cached = "libx264";
}

MediaInfo probeMedia(const QString& path) {
    MediaInfo m;
    QProcess p;
    p.start(ffmpegPath(), {"-hide_banner", "-i", path});
    if (!p.waitForFinished(20000)) return m;
    const QString e = QString::fromUtf8(p.readAllStandardError());
    auto d = QRegularExpression(R"(Duration:\s*(\d+):(\d+):(\d+(?:\.\d+)?))").match(e);
    if (d.hasMatch()) {
        m.duration = d.captured(1).toInt() * 3600 + d.captured(2).toInt() * 60 + d.captured(3).toDouble();
        m.ok = true;
    }
    m.hasAudio = e.contains(QRegularExpression(R"(Stream #\d+:\d+.*Audio:)"));
    auto v = QRegularExpression(R"(Stream #\d+:\d+.*Video:.*?(\d{2,5})x(\d{2,5}))").match(e);
    if (v.hasMatch()) { m.w = v.captured(1).toInt(); m.h = v.captured(2).toInt(); }
    auto f = QRegularExpression(R"((\d+(?:\.\d+)?) fps)").match(e);
    if (f.hasMatch()) m.fps = f.captured(1).toDouble();
    return m;
}

static QString atempoChain(double sp) {
    QStringList parts;
    while (sp > 2.0) { parts << "atempo=2.0"; sp /= 2.0; }
    while (sp < 0.5) { parts << "atempo=0.5"; sp /= 0.5; }
    parts << QString("atempo=%1").arg(sp, 0, 'f', 5);
    return parts.join(",");
}

static QStringList encArgs(const QString& enc, int q) {
    static const int qv[3] = {19, 24, 29};
    const QString v = QString::number(qv[std::clamp(q, 0, 2)]);
    if (enc == "h264_nvenc") return {"-c:v", enc, "-preset", "p5", "-rc", "vbr", "-cq", v, "-b:v", "0"};
    if (enc == "h264_videotoolbox") {
        static const char* qq[3] = {"75", "60", "45"};
        return {"-c:v", enc, "-q:v", qq[std::clamp(q, 0, 2)]};
    }
    if (enc == "h264_qsv") return {"-c:v", enc, "-preset", "medium", "-global_quality", v};
    if (enc == "h264_amf")
        return {"-c:v", enc, "-quality", "quality", "-rc", "cqp", "-qp_i", v, "-qp_p", QString::number(v.toInt() + 2),
                "-qp_b", QString::number(v.toInt() + 4)};
    static const int crf[3] = {18, 23, 28};
    return {"-c:v", "libx264", "-preset", "veryfast", "-crf", QString::number(crf[std::clamp(q, 0, 2)])};
}

QStringList buildExport(const Project& pr, const ExportOptions& o, const QString& out) {
    auto n = [](double v, int d = 4) { return QString::number(v, 'f', d); };
    const double total = pr.total();
    const int W = o.width, H = o.height;
    QStringList cmd = {"-hide_banner", "-y"};

    // Eingänge: Quellen, dann Bilder, dann Ton-Clips
    const int S = pr.sources.size();
    for (const Source& s : pr.sources) cmd << "-i" << s.path;
    int nextIn = S;
    QList<int> imgIn;
    for (const Item& it : pr.items) {
        if (it.kind == Item::Image) {
            cmd << "-loop" << "1" << "-framerate" << n(o.fps, 3) << "-t" << n(total, 3) << "-i" << it.path;
            imgIn << nextIn++;
        }
    }
    QList<int> audIn;
    for (const AudioClip& a : pr.audios) { cmd << "-i" << a.path; audIn << nextIn++; }

    bool videoAudio = false;
    for (const Piece& p : pr.pieces) videoAudio |= pr.sources.value(p.src).hasAudio;
    const bool needAudio = videoAudio || !pr.audios.isEmpty();

    QStringList f;
    const int cnt = pr.pieces.size();
    for (int i = 0; i < cnt; ++i) {
        const Piece& p = pr.pieces[i];
        f << QString("[%1:v]trim=start=%2:end=%3,setpts=(PTS-STARTPTS)/%4,"
                     "scale=%5:%6:force_original_aspect_ratio=decrease,pad=%5:%6:(ow-iw)/2:(oh-ih)/2:black,"
                     "setsar=1,fps=%7[v%8]")
                 .arg(p.src).arg(n(p.start)).arg(n(p.end)).arg(n(p.speed, 5)).arg(W).arg(H).arg(n(o.fps, 3)).arg(i);
        if (videoAudio) {
            if (pr.sources.value(p.src).hasAudio)
                f << QString("[%1:a]atrim=start=%2:end=%3,asetpts=PTS-STARTPTS,%4,aresample=48000,"
                             "aformat=sample_fmts=fltp:channel_layouts=stereo[a%5]")
                         .arg(p.src).arg(n(p.start)).arg(n(p.end)).arg(atempoChain(p.speed)).arg(i);
            else
                f << QString("anullsrc=r=48000:cl=stereo:d=%1,aformat=sample_fmts=fltp[a%2]").arg(n(p.outDur())).arg(i);
        }
    }
    QString ins;
    for (int i = 0; i < cnt; ++i) ins += QString("[v%1]").arg(i) + (videoAudio ? QString("[a%1]").arg(i) : QString());
    f << QString("%1concat=n=%2:v=1:a=%3[vc]%4").arg(ins).arg(cnt).arg(videoAudio ? 1 : 0).arg(videoAudio ? "[abase]" : "");

    // Bilder + Unschärfe
    QString cur = "vc";
    int k = 0, imgN = 0;
    for (const Item& it : pr.items) {
        if (it.t1 <= it.t0) { if (it.kind == Item::Image) ++imgN; continue; }
        int x = int(it.x * W), y = int(it.y * H);
        int w = std::max(2, int(it.w * W)), h = std::max(2, int(it.h * H));
        QString en = QString("enable='between(t,%1,%2)'").arg(n(it.t0, 3)).arg(n(it.t1, 3));
        ++k;
        if (it.kind == Item::Blur) {
            x = std::clamp(x, 0, W - 2);
            y = std::clamp(y, 0, H - 2);
            w = std::min(w, W - x);
            h = std::min(h, H - y);
            const double sigma = it.strength * H / 1080.0;
            f << QString("[%1]split[s%2a][s%2b];[s%2b]crop=%3:%4:%5:%6,gblur=sigma=%7:steps=2[bl%2];"
                         "[s%2a][bl%2]overlay=%5:%6:%8[o%2]")
                     .arg(cur).arg(k).arg(w).arg(h).arg(x).arg(y).arg(n(std::max(1.0, sigma), 1)).arg(en);
        } else {
            f << QString("[%1:v]format=rgba,scale=%2:%3[im%4];[%5][im%4]overlay=%6:%7:%8[o%4]")
                     .arg(imgIn.value(imgN)).arg(w).arg(h).arg(k).arg(cur).arg(x).arg(y).arg(en);
            ++imgN;
        }
        cur = QString("o%1").arg(k);
    }
    f << QString("[%1]format=yuv420p[vout]").arg(cur);

    // Ton: Basis (Video-Ton oder Stille) + Ton-Clips mischen
    if (needAudio) {
        if (!videoAudio)
            f << QString("anullsrc=r=48000:cl=stereo:d=%1,aformat=sample_fmts=fltp[abase]").arg(n(total, 3));
        QString mix = "[abase]";
        for (int i = 0; i < pr.audios.size(); ++i) {
            const AudioClip& a = pr.audios[i];
            f << QString("[%1:a]atrim=start=%2:end=%3,asetpts=PTS-STARTPTS,volume=%4,aresample=48000,"
                         "aformat=sample_fmts=fltp:channel_layouts=stereo,adelay=%5:all=1[c%6]")
                     .arg(audIn[i]).arg(n(a.srcStart)).arg(n(a.srcStart + a.dur)).arg(n(a.volume, 3))
                     .arg(int(a.t0 * 1000)).arg(i);
            mix += QString("[c%1]").arg(i);
        }
        if (pr.audios.isEmpty()) f.replaceInStrings("[abase]", "[aout]");
        else f << QString("%1amix=inputs=%2:duration=first:normalize=0[aout]").arg(mix).arg(pr.audios.size() + 1);
    }

    cmd << "-filter_complex" << f.join(";") << "-map" << "[vout]";
    if (needAudio) cmd << "-map" << "[aout]";
    cmd << "-r" << n(o.fps, 3) << encArgs(o.encoder, o.quality);
    if (needAudio) cmd << "-c:a" << "aac" << "-b:a" << "192k";
    cmd << "-movflags" << "+faststart" << "-progress" << "pipe:1" << "-nostats" << out;
    return cmd;
}
