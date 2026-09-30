#include "i18n.h"

#include <QHash>

namespace {
constexpr int N = 12;
struct Entry { const char* key; const char* t[N]; };

const char* kCodes[N] = {"de", "en", "es", "fr", "it", "pt", "nl", "pl", "tr", "ru", "ja", "zh"};
const char* kNames[N] = {"Deutsch", "English", "Español", "Français", "Italiano", "Português",
                         "Nederlands", "Polski", "Türkçe", "Русский", "日本語", "中文"};

const Entry kTable[] = {
 {"save_to", {"Speichern unter","Save to","Guardar en","Enregistrer sous","Salva in","Guardar em","Opslaan in","Zapisz jako","Kaydet","Сохранить как","保存先","保存到"}},
 {"file_name", {"Dateiname","File name","Nombre del archivo","Nom du fichier","Nome del file","Nome do ficheiro","Bestandsnaam","Nazwa pliku","Dosya adı","Имя файла","ファイル名","文件名"}},
 {"folder", {"Speicherort","Location","Ubicación","Emplacement","Posizione","Localização","Locatie","Lokalizacja","Konum","Расположение","保存場所","位置"}},
 {"browse", {"Durchsuchen…","Browse…","Examinar…","Parcourir…","Sfoglia…","Procurar…","Bladeren…","Przeglądaj…","Gözat…","Обзор…","参照…","浏览…"}},
 {"replace_orig", {"Originaldatei ersetzen","Replace original file","Reemplazar el archivo original","Remplacer le fichier d'origine","Sostituisci il file originale","Substituir o ficheiro original","Origineel bestand vervangen","Zastąp oryginalny plik","Orijinal dosyanın yerine koy","Заменить исходный файл","元のファイルを置き換える","替换原始文件"}},
 {"replace_hint", {"Das Original wird erst nach erfolgreichem Export überschrieben.","The original is only overwritten after a successful export.","El original solo se sobrescribe tras una exportación correcta.","L'original n'est remplacé qu'après un export réussi.","L'originale viene sovrascritto solo dopo un'esportazione riuscita.","O original só é substituído após uma exportação bem-sucedida.","Het origineel wordt pas na een geslaagde export overschreven.","Oryginał zostanie nadpisany dopiero po udanym eksporcie.","Orijinal, yalnızca dışa aktarma başarılı olursa değiştirilir.","Оригинал перезаписывается только после успешного экспорта.","元のファイルは書き出しに成功した後にのみ上書きされます。","仅在导出成功后才会覆盖原文件。"}},
 {"overwrite_q", {"„%1“ gibt es schon. Überschreiben?","\"%1\" already exists. Overwrite it?","\"%1\" ya existe. ¿Sobrescribir?","« %1 » existe déjà. Le remplacer ?","\"%1\" esiste già. Sovrascrivere?","\"%1\" já existe. Substituir?","\"%1\" bestaat al. Overschrijven?","„%1” już istnieje. Zastąpić?","\"%1\" zaten var. Üzerine yazılsın mı?","«%1» уже существует. Заменить?","「%1」は既に存在します。上書きしますか？","\"%1\" 已存在。要覆盖吗？"}},
 {"fs_exit_hint", {"Esc oder F11 zum Beenden des Vollbilds","Press Esc or F11 to exit fullscreen","Pulsa Esc o F11 para salir de la pantalla completa","Appuyez sur Échap ou F11 pour quitter le plein écran","Premi Esc o F11 per uscire dallo schermo intero","Prima Esc ou F11 para sair do ecrã inteiro","Druk op Esc of F11 om volledig scherm te verlaten","Naciśnij Esc lub F11, aby wyjść z pełnego ekranu","Tam ekrandan çıkmak için Esc veya F11","Нажмите Esc или F11, чтобы выйти из полноэкранного режима","Esc または F11 で全画面を終了","按 Esc 或 F11 退出全屏"}},
 {"upd_title", {"Update verfügbar","Update available","Actualización disponible","Mise à jour disponible","Aggiornamento disponibile","Atualização disponível","Update beschikbaar","Dostępna aktualizacja","Güncelleme mevcut","Доступно обновление","アップデートがあります","有可用更新"}},
 {"upd_text", {"%1 %2 ist verfügbar (du hast %3). Jetzt updaten? Danach startet das Programm neu.","%1 %2 is available (you have %3). Update now? The app restarts afterwards.","%1 %2 está disponible (tienes %3). ¿Actualizar ahora? La aplicación se reiniciará.","%1 %2 est disponible (vous avez %3). Mettre à jour maintenant ? L'application redémarrera.","%1 %2 è disponibile (hai la %3). Aggiornare ora? L'app si riavvierà.","%1 %2 está disponível (tem a %3). Atualizar agora? A aplicação reinicia a seguir.","%1 %2 is beschikbaar (je hebt %3). Nu bijwerken? Daarna start de app opnieuw.","%1 %2 jest dostępny (masz %3). Zaktualizować teraz? Program uruchomi się ponownie.","%1 %2 mevcut (sizdeki: %3). Şimdi güncellensin mi? Ardından uygulama yeniden başlar.","Доступна версия %1 %2 (у вас %3). Обновить сейчас? Затем программа перезапустится.","%1 %2 が利用できます（現在 %3）。今すぐ更新しますか？更新後に再起動します。","%1 %2 已发布（当前 %3）。现在更新吗？更新后程序将重新启动。"}},
 {"upd_now", {"Jetzt updaten","Update now","Actualizar ahora","Mettre à jour","Aggiorna ora","Atualizar agora","Nu bijwerken","Aktualizuj teraz","Şimdi güncelle","Обновить сейчас","今すぐ更新","立即更新"}},
 {"upd_ignore", {"Dieses Update ignorieren","Ignore this update","Ignorar esta actualización","Ignorer cette mise à jour","Ignora questo aggiornamento","Ignorar esta atualização","Deze update negeren","Pomiń tę aktualizację","Bu güncellemeyi yoksay","Пропустить это обновление","このアップデートを無視","忽略此更新"}},
 {"upd_later", {"Später","Later","Más tarde","Plus tard","Più tardi","Mais tarde","Later","Później","Sonra","Позже","後で","稍后"}},
 {"upd_downloading", {"Update wird heruntergeladen…","Downloading update…","Descargando actualización…","Téléchargement de la mise à jour…","Download dell'aggiornamento…","A transferir atualização…","Update downloaden…","Pobieranie aktualizacji…","Güncelleme indiriliyor…","Загрузка обновления…","アップデートをダウンロード中…","正在下载更新…"}},
 {"upd_failed", {"Das Update konnte nicht automatisch installiert werden. Stattdessen öffnet sich die Download-Seite.","The update couldn't be installed automatically. The download page opens instead.","No se pudo instalar la actualización automáticamente. Se abrirá la página de descarga.","La mise à jour n'a pas pu être installée automatiquement. La page de téléchargement va s'ouvrir.","Impossibile installare l'aggiornamento automaticamente. Si apre la pagina di download.","Não foi possível instalar a atualização automaticamente. Abre-se a página de transferência.","De update kon niet automatisch worden geïnstalleerd. De downloadpagina wordt geopend.","Nie udało się zainstalować aktualizacji automatycznie. Otworzy się strona pobierania.","Güncelleme otomatik yüklenemedi. Bunun yerine indirme sayfası açılıyor.","Не удалось установить обновление автоматически. Откроется страница загрузки.","自動でインストールできませんでした。ダウンロードページを開きます。","无法自动安装更新，将打开下载页面。"}},
 {"upd_latest", {"Du hast die neueste Version.","You're using the latest version.","Tienes la versión más reciente.","Vous avez la dernière version.","Hai la versione più recente.","Tem a versão mais recente.","Je hebt de nieuwste versie.","Masz najnowszą wersję.","En son sürümü kullanıyorsunuz.","У вас последняя версия.","最新バージョンです。","已是最新版本。"}},
 {"upd_check", {"Nach Updates suchen","Check for updates","Buscar actualizaciones","Rechercher des mises à jour","Controlla aggiornamenti","Procurar atualizações","Controleren op updates","Sprawdź aktualizacje","Güncellemeleri denetle","Проверить обновления","アップデートを確認","检查更新"}},
 {"upd_unsaved", {"Deine aktuellen Änderungen gehen verloren. Trotzdem updaten?","Your current edits will be lost. Update anyway?","Se perderán los cambios actuales. ¿Actualizar de todos modos?","Vos modifications en cours seront perdues. Mettre à jour quand même ?","Le modifiche attuali andranno perse. Aggiornare comunque?","As alterações atuais serão perdidas. Atualizar mesmo assim?","Je huidige bewerkingen gaan verloren. Toch bijwerken?","Bieżące zmiany zostaną utracone. Mimo to zaktualizować?","Mevcut düzenlemeleriniz kaybolacak. Yine de güncellensin mi?","Текущие изменения будут потеряны. Всё равно обновить?","現在の編集内容は失われます。更新しますか？","当前编辑将会丢失。仍要更新吗？"}},
 {"media", {"Medien","Media","Medios","Médias","Media","Multimédia","Media","Multimedia","Medya","Медиа","メディア","媒体"}},
 {"import", {"Importieren","Import","Importar","Importer","Importa","Importar","Importeren","Importuj","İçe aktar","Импорт","読み込む","导入"}},
 {"properties", {"Eigenschaften","Properties","Propiedades","Propriétés","Proprietà","Propriedades","Eigenschappen","Właściwości","Özellikler","Свойства","プロパティ","属性"}},
 {"bin_hint", {"Doppelklick fügt die Datei an der Abspielposition ein.","Double-click a file to insert it at the playhead.","Haz doble clic para insertar en la posición actual.","Double-cliquez pour insérer à la position actuelle.","Doppio clic per inserire nella posizione attuale.","Clique duas vezes para inserir na posição atual.","Dubbelklik om in te voegen op de huidige positie.","Kliknij dwukrotnie, aby wstawić w bieżącym miejscu.","Geçerli konuma eklemek için çift tıklayın.","Дважды щёлкните, чтобы вставить в текущую позицию.","ダブルクリックで現在位置に挿入します。","双击即可插入到当前位置。"}},
 {"bin_empty", {"Noch keine Medien – auf „Importieren“ klicken oder Dateien hierher ziehen.","No media yet – click Import or drop files here.","Aún no hay medios: pulsa Importar o arrastra archivos aquí.","Aucun média – cliquez sur Importer ou glissez des fichiers ici.","Nessun media: premi Importa o trascina qui i file.","Ainda sem multimédia – clique em Importar ou arraste ficheiros.","Nog geen media – klik op Importeren of sleep bestanden hierheen.","Brak multimediów – kliknij Importuj lub przeciągnij pliki.","Henüz medya yok – İçe aktar'a tıklayın veya dosya sürükleyin.","Медиа пока нет – нажмите «Импорт» или перетащите файлы.","メディアがありません。「読み込む」を押すかファイルをドロップしてください。","暂无媒体 – 点击“导入”或将文件拖到此处。"}},
 {"text", {"Text","Text","Texto","Texte","Testo","Texto","Tekst","Tekst","Metin","Текст","テキスト","文本"}},
 {"text_ph", {"Dein Text","Your text","Tu texto","Votre texte","Il tuo testo","O seu texto","Jouw tekst","Twój tekst","Metniniz","Ваш текст","テキストを入力","你的文字"}},
 {"font", {"Schriftart","Font","Fuente","Police","Carattere","Tipo de letra","Lettertype","Czcionka","Yazı tipi","Шрифт","フォント","字体"}},
 {"color", {"Farbe","Color","Color","Couleur","Colore","Cor","Kleur","Kolor","Renk","Цвет","色","颜色"}},
 {"background", {"Hintergrund","Background","Fondo","Fond","Sfondo","Fundo","Achtergrond","Tło","Arka plan","Фон","背景","背景"}},
 {"position", {"Position","Position","Posición","Position","Posizione","Posição","Positie","Pozycja","Konum","Положение","位置","位置"}},
 {"rotation", {"Drehen","Rotation","Rotación","Rotation","Rotazione","Rotação","Rotatie","Obrót","Döndürme","Поворот","回転","旋转"}},
 {"reset", {"Zurücksetzen","Reset","Restablecer","Réinitialiser","Ripristina","Repor","Herstellen","Resetuj","Sıfırla","Сбросить","リセット","重置"}},
 {"zoom_in", {"Heranzoomen","Zoom in","Acercar","Zoom avant","Ingrandisci","Aproximar","Inzoomen","Powiększ","Yakınlaştır","Приблизить","拡大","放大"}},
 {"zoom_out", {"Herauszoomen","Zoom out","Alejar","Zoom arrière","Riduci","Afastar","Uitzoomen","Pomniejsz","Uzaklaştır","Отдалить","縮小","缩小"}},
 {"format", {"Format","Format","Formato","Format","Formato","Formato","Formaat","Format","Biçim","Формат","形式","格式"}},
 {"audio_only", {"nur Ton","audio only","solo audio","audio seul","solo audio","só áudio","alleen audio","tylko dźwięk","yalnızca ses","только звук","音声のみ","仅音频"}},
 {"open", {"Öffnen","Open","Abrir","Ouvrir","Apri","Abrir","Openen","Otwórz","Aç","Открыть","開く","打开"}},
 {"add_media", {"Medien hinzufügen","Add media","Añadir medios","Ajouter un média","Aggiungi media","Adicionar multimédia","Media toevoegen","Dodaj multimedia","Medya ekle","Добавить медиа","メディアを追加","添加媒体"}},
 {"add_media_tip", {"Video oder Ton an der aktuellen Stelle einfügen","Insert a video or audio file at the playhead","Insertar un vídeo o audio en la posición actual","Insérer une vidéo ou un son à la position actuelle","Inserisci un video o un audio nella posizione attuale","Inserir vídeo ou áudio na posição atual","Video of audio invoegen op de huidige positie","Wstaw wideo lub dźwięk w bieżącym miejscu","Geçerli konuma video veya ses ekle","Вставить видео или звук в текущую позицию","現在位置に動画または音声を挿入","在当前位置插入视频或音频"}},
 {"media_files", {"Video und Ton","Video and audio","Vídeo y audio","Vidéo et son","Video e audio","Vídeo e áudio","Video en audio","Wideo i dźwięk","Video ve ses","Видео и звук","動画と音声","视频和音频"}},
 {"media_unreadable", {"Diese Datei kann nicht gelesen werden.","This file can't be read.","No se puede leer este archivo.","Impossible de lire ce fichier.","Impossibile leggere questo file.","Não é possível ler este ficheiro.","Dit bestand kan niet worden gelezen.","Nie można odczytać tego pliku.","Bu dosya okunamıyor.","Не удаётся прочитать этот файл.","このファイルは読み込めません。","无法读取此文件。"}},
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
 {"aspect", {"Format:","Format:","Formato:","Format :","Formato:","Formato:","Formaat:","Format:","Format:","Формат:","画面比率:","画幅:"}},
 {"aspect_tip", {"Bildformat des fertigen Videos – das Video wird eingepasst, der Rest bleibt schwarz","Frame format of the finished video – the video is fitted in, the rest stays black","Formato del vídeo final: el vídeo se ajusta y el resto queda en negro","Format de la vidéo finale : la vidéo est ajustée, le reste reste noir","Formato del video finale: il video viene adattato, il resto resta nero","Formato do vídeo final: o vídeo é ajustado, o resto fica preto","Beeldformaat van de uiteindelijke video – de video wordt ingepast, de rest blijft zwart","Format gotowego filmu – wideo zostanie dopasowane, reszta pozostanie czarna","Bitmiş videonun formatı – video sığdırılır, kalan kısım siyah kalır","Формат готового видео – видео вписывается, остальное остаётся чёрным","完成した動画の画面比率 – 動画は収まるように配置され、残りは黒になります","成片画幅 – 视频会适配画面，其余部分为黑色"}},
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
