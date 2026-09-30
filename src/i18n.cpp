#include "i18n.h"

#include <QHash>

namespace {
constexpr int N = 12;
struct Entry { const char* key; const char* t[N]; };

const char* kCodes[N] = {"de", "en", "es", "fr", "it", "pt", "nl", "pl", "tr", "ru", "ja", "zh"};
const char* kNames[N] = {"Deutsch", "English", "Español", "Français", "Italiano", "Português",
                         "Nederlands", "Polski", "Türkçe", "Русский", "日本語", "中文"};

const Entry kTable[] = {
 {"open", {"Öffnen","Open","Abrir","Ouvrir","Apri","Abrir","Openen","Otwórz","Aç","Открыть","開く","打开"}},
 {"add_video", {"Video hinzufügen","Add video","Añadir vídeo","Ajouter une vidéo","Aggiungi video","Adicionar vídeo","Video toevoegen","Dodaj wideo","Video ekle","Добавить видео","動画を追加","添加视频"}},
 {"add_audio", {"Ton hinzufügen","Add audio","Añadir audio","Ajouter un son","Aggiungi audio","Adicionar áudio","Audio toevoegen","Dodaj dźwięk","Ses ekle","Добавить звук","音声を追加","添加音频"}},
 {"edit", {"Bearbeiten","Edit","Editar","Modifier","Modifica","Editar","Bewerken","Edytuj","Düzenle","Правка","編集","编辑"}},
 {"export", {"Exportieren","Export","Exportar","Exporter","Esporta","Exportar","Exporteren","Eksportuj","Dışa aktar","Экспорт","書き出し","导出"}},
 {"settings", {"Einstellungen","Settings","Ajustes","Paramètres","Impostazioni","Definições","Instellingen","Ustawienia","Ayarlar","Настройки","設定","设置"}},
 {"play_tip", {"Abspielen / Pause (Leertaste)","Play / Pause (Space)","Reproducir / Pausa (Espacio)","Lecture / Pause (Espace)","Riproduci / Pausa (Spazio)","Reproduzir / Pausa (Espaço)","Afspelen / Pauzeren (Spatie)","Odtwórz / Pauza (Spacja)","Oynat / Duraklat (Boşluk)","Воспроизведение / Пауза (Пробел)","再生 / 一時停止 (スペース)","播放 / 暂停（空格）"}},
 {"fullscreen_tip", {"Vollbild (F11)","Fullscreen (F11)","Pantalla completa (F11)","Plein écran (F11)","Schermo intero (F11)","Ecrã inteiro (F11)","Volledig scherm (F11)","Pełny ekran (F11)","Tam ekran (F11)","Полный экран (F11)","全画面 (F11)","全屏 (F11)"}},
 {"split", {"Teilen","Split","Dividir","Diviser","Dividi","Dividir","Splitsen","Podziel","Böl","Разрезать","分割","分割"}},
 {"remove_piece", {"Abschnitt entfernen","Remove segment","Eliminar tramo","Supprimer le segment","Rimuovi segmento","Remover segmento","Segment verwijderen","Usuń fragment","Bölümü sil","Удалить фрагмент","区間を削除","删除片段"}},
 {"trim_start", {"Anfang kürzen","Trim start","Recortar inicio","Couper le début","Taglia inizio","Cortar início","Begin inkorten","Przytnij początek","Başı kırp","Обрезать начало","先頭をカット","裁剪开头"}},
 {"trim_end", {"Ende kürzen","Trim end","Recortar final","Couper la fin","Taglia fine","Cortar fim","Einde inkorten","Przytnij koniec","Sonu kırp","Обрезать конец","末尾をカット","裁剪结尾"}},
 {"blur", {"Unscharf","Blur","Desenfoque","Flou","Sfocatura","Desfoque","Vervagen","Rozmycie","Bulanık","Размытие","ぼかし","模糊"}},
 {"image", {"Bild","Image","Imagen","Image","Immagine","Imagem","Afbeelding","Obraz","Resim","Изображение","画像","图片"}},
 {"undo", {"Rückgängig (Strg+Z)","Undo (Ctrl+Z)","Deshacer (Ctrl+Z)","Annuler (Ctrl+Z)","Annulla (Ctrl+Z)","Anular (Ctrl+Z)","Ongedaan maken (Ctrl+Z)","Cofnij (Ctrl+Z)","Geri al (Ctrl+Z)","Отменить (Ctrl+Z)","元に戻す (Ctrl+Z)","撤销 (Ctrl+Z)"}},
 {"redo", {"Wiederholen (Strg+Y)","Redo (Ctrl+Y)","Rehacer (Ctrl+Y)","Rétablir (Ctrl+Y)","Ripeti (Ctrl+Y)","Refazer (Ctrl+Y)","Opnieuw (Ctrl+Y)","Ponów (Ctrl+Y)","Yinele (Ctrl+Y)","Повторить (Ctrl+Y)","やり直す (Ctrl+Y)","重做 (Ctrl+Y)"}},
 {"drop_hint", {"Video hierher ziehen oder „Öffnen“ klicken (Strg+O)","Drop a video here or click Open (Ctrl+O)","Arrastra un vídeo aquí o pulsa Abrir (Ctrl+O)","Glissez une vidéo ici ou cliquez sur Ouvrir (Ctrl+O)","Trascina un video qui o premi Apri (Ctrl+O)","Arraste um vídeo para aqui ou clique em Abrir (Ctrl+O)","Sleep een video hierheen of klik op Openen (Ctrl+O)","Przeciągnij tu wideo lub kliknij Otwórz (Ctrl+O)","Bir videoyu buraya sürükleyin veya Aç'a tıklayın (Ctrl+O)","Перетащите видео сюда или нажмите «Открыть» (Ctrl+O)","動画をここにドロップするか「開く」を押してください (Ctrl+O)","将视频拖到此处，或点击“打开”(Ctrl+O)"}},
 {"insp_hint", {"Abschnitt in der Timeline anklicken oder ein Element wählen.","Click a segment on the timeline or select an element.","Haz clic en un tramo de la línea de tiempo o elige un elemento.","Cliquez sur un segment de la timeline ou choisissez un élément.","Fai clic su un segmento della timeline o scegli un elemento.","Clique num segmento da linha do tempo ou escolha um elemento.","Klik op een segment in de tijdlijn of kies een element.","Kliknij fragment na osi czasu lub wybierz element.","Zaman çizelgesinde bir bölüme tıklayın veya bir öğe seçin.","Выберите фрагмент на таймлайне или элемент.","タイムラインの区間または要素を選択してください。","请在时间轴上选择片段或元素。"}},
 {"segment", {"Abschnitt","Segment","Tramo","Segment","Segmento","Segmento","Segment","Fragment","Bölüm","Фрагмент","区間","片段"}},
 {"speed", {"Tempo","Speed","Velocidad","Vitesse","Velocità","Velocidade","Snelheid","Prędkość","Hız","Скорость","速度","速度"}},
 {"source_range", {"Quelle %1 – %2","Source %1 – %2","Origen %1 – %2","Source %1 – %2","Origine %1 – %2","Origem %1 – %2","Bron %1 – %2","Źródło %1 – %2","Kaynak %1 – %2","Источник %1 – %2","元 %1 – %2","源 %1 – %2"}},
 {"result_dur", {"Dauer im Ergebnis: %1","Duration in result: %1","Duración en el resultado: %1","Durée dans le résultat : %1","Durata nel risultato: %1","Duração no resultado: %1","Duur in resultaat: %1","Czas w wyniku: %1","Sonuçtaki süre: %1","Длительность в результате: %1","結果での長さ: %1","结果时长：%1"}},
 {"blur_area", {"Unscharfer Bereich","Blurred area","Zona desenfocada","Zone floutée","Area sfocata","Área desfocada","Vervaagd gebied","Rozmyty obszar","Bulanık alan","Область размытия","ぼかし範囲","模糊区域"}},
 {"audio_el", {"Ton","Audio","Audio","Son","Audio","Áudio","Audio","Dźwięk","Ses","Звук","音声","音频"}},
 {"start_s", {"Start (s)","Start (s)","Inicio (s)","Début (s)","Inizio (s)","Início (s)","Start (s)","Start (s)","Başlangıç (sn)","Начало (с)","開始 (秒)","开始（秒）"}},
 {"end_s", {"Ende (s)","End (s)","Fin (s)","Fin (s)","Fine (s)","Fim (s)","Einde (s)","Koniec (s)","Bitiş (sn)","Конец (с)","終了 (秒)","结束（秒）"}},
 {"size", {"Größe","Size","Tamaño","Taille","Dimensione","Tamanho","Grootte","Rozmiar","Boyut","Размер","サイズ","大小"}},
 {"strength", {"Stärke","Strength","Intensidad","Intensité","Intensità","Intensidade","Sterkte","Siła","Güç","Сила","強さ","强度"}},
 {"volume", {"Lautstärke","Volume","Volumen","Volume","Volume","Volume","Volume","Głośność","Ses düzeyi","Громкость","音量","音量"}},
 {"delete_el", {"Element löschen","Delete element","Eliminar elemento","Supprimer l'élément","Elimina elemento","Eliminar elemento","Element verwijderen","Usuń element","Öğeyi sil","Удалить элемент","要素を削除","删除元素"}},
 {"resolution", {"Auflösung","Resolution","Resolución","Résolution","Risoluzione","Resolução","Resolutie","Rozdzielczość","Çözünürlük","Разрешение","解像度","分辨率"}},
 {"framerate", {"Bildrate","Frame rate","Fotogramas por segundo","Images par seconde","Fotogrammi al secondo","Fotogramas por segundo","Beelden per seconde","Klatki na sekundę","Kare hızı","Частота кадров","フレームレート","帧率"}},
 {"quality", {"Qualität","Quality","Calidad","Qualité","Qualità","Qualidade","Kwaliteit","Jakość","Kalite","Качество","画質","质量"}},
 {"q_high", {"Hoch","High","Alta","Élevée","Alta","Alta","Hoog","Wysoka","Yüksek","Высокое","高","高"}},
 {"q_mid", {"Ausgewogen","Balanced","Equilibrada","Équilibrée","Bilanciata","Equilibrada","Gebalanceerd","Zrównoważona","Dengeli","Баланс","標準","均衡"}},
 {"q_small", {"Klein (spart Speicher)","Small (saves space)","Pequeña (ahorra espacio)","Petite (économise de l'espace)","Piccola (risparmia spazio)","Pequena (poupa espaço)","Klein (bespaart ruimte)","Mała (oszczędza miejsce)","Küçük (yer kazandırır)","Малое (экономит место)","小 (容量節約)","小（节省空间）"}},
 {"original", {"Original","Original","Original","Original","Originale","Original","Origineel","Oryginał","Orijinal","Оригинал","オリジナル","原始"}},
 {"export_hint", {"Kleinere Werte sparen Speicherplatz. Höher als das Original wird nicht angeboten.","Lower values save storage. Anything above the original is not offered.","Los valores menores ahorran espacio. No se ofrece más que el original.","Des valeurs plus basses économisent de l'espace. Rien au-dessus de l'original.","Valori più bassi risparmiano spazio. Non si offre più dell'originale.","Valores menores poupam espaço. Não se oferece acima do original.","Lagere waarden besparen ruimte. Hoger dan het origineel wordt niet aangeboden.","Niższe wartości oszczędzają miejsce. Wyższych niż oryginał nie oferujemy.","Düşük değerler yer kazandırır. Orijinalden yükseği sunulmaz.","Меньшие значения экономят место. Выше оригинала не предлагается.","小さい値は容量を節約できます。元より大きい値は選べません。","较低的值可节省空间，不提供高于原始的选项。"}},
 {"done", {"Fertig","Done","Listo","Terminé","Fatto","Concluído","Klaar","Gotowe","Tamam","Готово","完了","完成"}},
 {"saved", {"Gespeichert:","Saved:","Guardado:","Enregistré :","Salvato:","Guardado:","Opgeslagen:","Zapisano:","Kaydedildi:","Сохранено:","保存しました:","已保存："}},
 {"export_fail", {"Export fehlgeschlagen","Export failed","Error al exportar","Échec de l'export","Esportazione non riuscita","Falha ao exportar","Export mislukt","Eksport nie powiódł się","Dışa aktarma başarısız","Ошибка экспорта","書き出しに失敗しました","导出失败"}},
 {"exporting", {"Exportiere …","Exporting …","Exportando …","Exportation …","Esportazione …","A exportar …","Exporteren …","Eksportowanie …","Dışa aktarılıyor …","Экспорт …","書き出し中 …","正在导出 …"}},
 {"cancel", {"Abbrechen","Cancel","Cancelar","Annuler","Annulla","Cancelar","Annuleren","Anuluj","İptal","Отмена","キャンセル","取消"}},
 {"ok", {"OK","OK","OK","OK","OK","OK","OK","OK","Tamam","OK","OK","确定"}},
 {"language", {"Sprache","Language","Idioma","Langue","Lingua","Idioma","Taal","Język","Dil","Язык","言語","语言"}},
 {"theme", {"Design","Theme","Tema","Thème","Tema","Tema","Thema","Motyw","Tema","Тема","テーマ","主题"}},
 {"choose_language", {"Sprache wählen","Choose your language","Elige tu idioma","Choisissez votre langue","Scegli la lingua","Escolha o idioma","Kies je taal","Wybierz język","Dilinizi seçin","Выберите язык","言語を選択","选择语言"}},
 {"open_video", {"Video öffnen","Open video","Abrir vídeo","Ouvrir une vidéo","Apri video","Abrir vídeo","Video openen","Otwórz wideo","Video aç","Открыть видео","動画を開く","打开视频"}},
 {"choose_image", {"Bild wählen","Choose image","Elegir imagen","Choisir une image","Scegli immagine","Escolher imagem","Afbeelding kiezen","Wybierz obraz","Resim seç","Выбрать изображение","画像を選択","选择图片"}},
 {"choose_audio", {"Ton wählen","Choose audio","Elegir audio","Choisir un son","Scegli audio","Escolher áudio","Audio kiezen","Wybierz dźwięk","Ses seç","Выбрать звук","音声を選択","选择音频"}},
};

int g_lang = 0;

const QHash<QString, const Entry*>& index() {
    static QHash<QString, const Entry*> h = [] {
        QHash<QString, const Entry*> m;
        for (const Entry& e : kTable) m.insert(e.key, &e);
        return m;
    }();
    return h;
}
}  // namespace

QStringList languageCodes() {
    QStringList l;
    for (const char* c : kCodes) l << c;
    return l;
}

QStringList languageNames() {
    QStringList l;
    for (const char* c : kNames) l << QString::fromUtf8(c);
    return l;
}

void setLanguage(const QString& code) {
    int i = languageCodes().indexOf(code);
    g_lang = i < 0 ? 1 : i;
}

QString currentLanguage() { return kCodes[g_lang]; }

QString T(const char* key) {
    auto it = index().constFind(QString::fromLatin1(key));
    if (it == index().constEnd()) return QString::fromLatin1(key);
    return QString::fromUtf8((*it)->t[g_lang]);
}
