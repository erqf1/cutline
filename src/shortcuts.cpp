#include "shortcuts.h"

#include <QSettings>

const QList<ShortcutDef>& shortcutDefs() {
    static const QList<ShortcutDef> defs = {
        {"play", "sc_play", "Space"},
        {"back", "sc_back", "Left"},
        {"fwd", "sc_fwd", "Right"},
        {"fullscreen", "sc_fullscreen", "F11"},
        {"exitfs", "sc_exitfs", "Esc"},
        {"bar", "sc_bar", "H"},
        {"edit", "edit", "E"},
        {"open", "open", "Ctrl+O"},
        {"export", "export", "Ctrl+E"},
        {"split", "split", "S"},
        {"delete", "remove_piece", "Del"},
        {"trim_start", "trim_start", "Q"},
        {"trim_end", "trim_end", "W"},
        {"blur", "blur", "B"},
        {"image", "image", "I"},
        {"text", "text", "T"},
        {"undo", "sc_undo", "Ctrl+Z"},
        {"redo", "sc_redo", "Ctrl+Y"},
        {"zoom_in", "zoom_in", "Ctrl++"},
        {"zoom_out", "zoom_out", "Ctrl+-"},
    };
    return defs;
}

QKeySequence defaultShortcut(const QString& id) {
    for (const ShortcutDef& d : shortcutDefs())
        if (id == d.id) return QKeySequence(QString::fromLatin1(d.def), QKeySequence::PortableText);
    return {};
}

QKeySequence shortcutFor(const QString& id) {
    QSettings st;
    const QString key = "shortcuts/" + id;
    if (!st.contains(key)) return defaultShortcut(id);
    return QKeySequence(st.value(key).toString(), QKeySequence::PortableText);
}

void setShortcut(const QString& id, const QKeySequence& seq) {
    QSettings st;
    if (seq == defaultShortcut(id)) st.remove("shortcuts/" + id);
    else st.setValue("shortcuts/" + id, seq.toString(QKeySequence::PortableText));
}

void resetShortcuts() { QSettings().remove("shortcuts"); }

QString shortcutText(const QString& id) { return shortcutFor(id).toString(QKeySequence::NativeText); }
