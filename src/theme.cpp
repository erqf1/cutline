#include "theme.h"

static QList<Theme> makeThemes() {
    auto mk = [](const char* id, const char* name, const char* bg, const char* panel, const char* panel2,
                 const char* border, const char* text, const char* muted, const char* accent, const char* acctext,
                 const char* tl, const char* video, const char* font = "'Segoe UI','SF Pro Text','Helvetica Neue','Ubuntu','Noto Sans','DejaVu Sans',sans-serif", int px = 13, int r = 8,
                 bool xp = false) {
        Theme t;
        t.id = id; t.name = name;
        t.bg = bg; t.panel = panel; t.panel2 = panel2; t.border = border; t.text = text; t.muted = muted;
        t.accent = accent; t.accentText = acctext; t.timeline = tl; t.video = video;
        t.font = font; t.fontPx = px; t.radius = r; t.xp = xp;
        return t;
    };
    return {
        mk("mint", "Mint", "#0d0f14", "#151821", "#1c202b", "#2a2f3d", "#e6e8ee", "#8b90a0", "#4ade9b", "#04231a",
           "#0a0c11", "#000000"),
        mk("ocean", "Ocean", "#0a101d", "#101a2e", "#16233c", "#263557", "#e3ecff", "#8fa3c7", "#4aa3ff", "#04101f",
           "#08101c", "#000000"),
        mk("violet", "Violett", "#100c1b", "#181227", "#221a36", "#392c59", "#efe9ff", "#9d8fc0", "#a78bfa", "#140a2e",
           "#0c0916", "#000000"),
        mk("sunset", "Sunset", "#17100e", "#221815", "#2d211c", "#493429", "#fbeee6", "#b59a8a", "#ff8a4c", "#2a1204",
           "#120c0a", "#000000"),
        mk("light", "Hell / Light", "#eef0f5", "#ffffff", "#eceef4", "#d3d7e0", "#1b1f2a", "#6a7083", "#2a9d6f",
           "#ffffff", "#e4e7ee", "#111111"),
        mk("xp", "Windows XP", "#ECE9D8", "#F7F6EE", "#FFFFFF", "#7F9DB9", "#000000", "#4d4d4d", "#316AC5", "#FFFFFF",
           "#FFFFFF", "#000000", "'Tahoma','Verdana','DejaVu Sans',sans-serif", 12, 3, true),
        mk("terminal", "Terminal", "#000000", "#04100a", "#08180f", "#14532d", "#7CFC9A", "#3f9c5a", "#22ff66",
           "#001a08", "#010a04", "#000000", "'Consolas','Menlo','DejaVu Sans Mono',monospace", 13, 0),
    };
}

const QList<Theme>& themes() {
    static const QList<Theme> t = makeThemes();
    return t;
}

static QString g_id = "mint";

const Theme& currentTheme() {
    for (const Theme& t : themes())
        if (t.id == g_id) return t;
    return themes().first();
}

void setCurrentTheme(const QString& id) { g_id = id; }

QString buildStyleSheet(const Theme& t) {
    QString s = R"(
* { font-family: @font@; font-size: @fs@px; color: @text@; }
QMainWindow, QDialog, QWidget#root { background: @bg@; }
QLabel { background: transparent; }
QLabel#hint { color: @muted@; }
QLabel#title { font-size: @fs2@px; font-weight: 600; }
QFrame#card { background: @panel@; border: 1px solid @border@; border-radius: @r@px; }
QFrame#card QLabel { background: transparent; }
QPushButton { background: @panel2@; border: 1px solid @border@; border-radius: @r@px; padding: 7px 12px; }
QPushButton:hover { border-color: @accent@; }
QPushButton:pressed { background: @border@; }
QPushButton:checked { background: @accent@; color: @acctext@; border-color: @accent@; }
QPushButton:disabled { color: @muted@; }
QPushButton#primary { background: @accent@; color: @acctext@; border-color: @accent@; font-weight: 600; }
QPushButton#primary:hover { background: @accent2@; }
QPushButton#play { border-radius: 20px; min-width: 40px; max-width: 40px; min-height: 40px; max-height: 40px;
                   padding: 0; background: @accent@; border-color: @accent@; }
QPushButton#play:hover { background: @accent2@; }
QPushButton#tool { padding: 6px 10px; }
QDoubleSpinBox, QSpinBox, QComboBox, QLineEdit { background: @panel2@; border: 1px solid @border@;
    border-radius: @r6@px; padding: 5px 8px; selection-background-color: @accent@; selection-color: @acctext@; }
QComboBox::drop-down { border: none; width: 22px; }
QComboBox QAbstractItemView { background: @panel@; border: 1px solid @border@; outline: 0;
    selection-background-color: @accent@; selection-color: @acctext@; }
QListWidget { background: @panel2@; border: 1px solid @border@; border-radius: @r@px; padding: 4px; outline: 0; }
QListWidget::item { padding: 9px 10px; border-radius: @r6@px; }
QListWidget::item:hover { background: @border@; }
QListWidget::item:selected { background: @accent@; color: @acctext@; }
QGraphicsView { border: none; border-radius: @r@px; background: @video@; }
QScrollArea { border: 1px solid @border@; border-radius: @r@px; background: @tl@; }
QScrollBar:horizontal { height: 10px; background: transparent; margin: 0; }
QScrollBar::handle:horizontal { background: @border@; border-radius: 5px; min-width: 30px; }
QScrollBar:vertical { width: 10px; background: transparent; margin: 0; }
QScrollBar::handle:vertical { background: @border@; border-radius: 5px; min-height: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
QSlider::groove:horizontal { height: 4px; background: @border@; border-radius: 2px; }
QSlider::sub-page:horizontal { background: @accent@; border-radius: 2px; }
QSlider::handle:horizontal { width: 14px; height: 14px; margin: -6px 0; background: @text@; border-radius: 7px; }
QToolTip { background: @panel@; color: @text@; border: 1px solid @border@; padding: 4px; }
QProgressBar { background: @panel2@; border: 1px solid @border@; border-radius: @r6@px; text-align: center; height: 16px; }
QProgressBar::chunk { background: @accent@; border-radius: @r6@px; }
)";
    if (t.xp) {
        s += R"(
QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #FFFFFF, stop:0.85 #ECEBE5, stop:1 #D6D0C5);
    border: 1px solid #003C74; border-radius: 3px; }
QPushButton:hover { border: 2px solid #F5B335; }
QPushButton:pressed { background: #D6D0C5; }
QPushButton:checked, QPushButton#play { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #5A8AE0, stop:1 #2A5BC0);
    color: #FFFFFF; border: 1px solid #003C74; }
QPushButton#primary { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #5DC24A, stop:1 #2E8B22);
    color: #FFFFFF; border: 1px solid #1B5E14; }
QFrame#card { background: #FFFFFF; border: 1px solid #7F9DB9; }
)";
    }
    auto hex = [](const QColor& c) { return c.name(); };
    s.replace("@font@", t.font).replace("@fs@", QString::number(t.fontPx)).replace("@fs2@", QString::number(t.fontPx + 3))
        .replace("@r@", QString::number(t.radius)).replace("@r6@", QString::number(std::max(0, t.radius - 3)))
        .replace("@bg@", hex(t.bg)).replace("@panel@", hex(t.panel)).replace("@panel2@", hex(t.panel2))
        .replace("@border@", hex(t.border)).replace("@text@", hex(t.text)).replace("@muted@", hex(t.muted))
        .replace("@accent2@", hex(t.accent.lighter(115))).replace("@accent@", hex(t.accent))
        .replace("@acctext@", hex(t.accentText)).replace("@tl@", hex(t.timeline)).replace("@video@", hex(t.video));
    return s;
}
