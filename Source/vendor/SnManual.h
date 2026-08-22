// [vendor] sn-lookandfeel 6f5aa33 sha256:604b23199ecdd78c
// This line and below are a distributed copy. Fix sn-lookandfeel and hand it out again
#pragma once
// SnManual.h - the "?" button, and the manual it hands to the reader.
//
// Distributed from sn-lookandfeel. Do not edit this copy; fix the one there
// and hand it out again.
//
// Why the manual is embedded, not shipped beside the binary
// ---------------------------------------------------------
// A file placed next to a plug-in has three ways to go missing, and not one of
// them raises an error:
//
//   - the reader moves or renames the folder;
//   - a recursive copy nests the bundle inside the old one, so the host loads
//     one build while the manual on disk belongs to another;
//   - File::currentExecutableFile is documented as "where possible" - JUCE
//     itself declines to guarantee it for plug-ins.
//
// Embedding removes the question. Wherever the .vst3 ends up, the manual is
// inside it, and it is the manual for THAT build. The cost is that changing
// the manual means rebuilding, which is the correct coupling: a manual that
// can drift away from its binary is a manual nobody can trust.
//
// What this file does NOT do
// --------------------------
// It does not check that the manual is current. That check belongs at build
// time, where it can stop the build - see tools/build_manual.py, which reads
// the parameters out of the built VST3 and fails if the prose and the plug-in
// disagree about what parameters exist. A runtime warning would arrive after
// the mistake had already shipped.
//
// Usage
// -----
//     #include "vendor/SnManual.h"
//
//     sn::HelpButton help;                          // a member of the editor
//
//     addAndMakeVisible (help);
//     help.onClick = [this]
//     {
//         auto why = sn::manual::open (BinaryData::manual_html,
//                                      BinaryData::manual_htmlSize,
//                                      BinaryData::manual_pdf,
//                                      BinaryData::manual_pdfSize,
//                                      "BitMod-8");
//         if (why.isNotEmpty())
//             sn::manual::report (this, why);
//     };
//
// One button, two files. The page is what opens; the PDF is laid down beside
// it so the "print this" link inside the page has something to point at. If
// the PDF could not be written the link 404s in the browser, which is at least
// a failure the reader can see - unlike a button that quietly does half its
// job.
//
//     // in resized():
//     help.setBounds (getWidth() - 30, 8, 22, 22);
//
// A "?" that silently does nothing is worse than no "?" at all, so open()
// returns the reason it failed instead of a bool, and report() shows it.

#include <juce_gui_basics/juce_gui_basics.h>

namespace sn
{
namespace manual
{

/** Where a product's extracted manual lives.

    A folder per product under the system temp directory. Per product, because
    every manual is called manual.html - the page links to manual.pdf by a
    fixed relative name, so two plug-ins sharing one folder would overwrite
    each other's. Under temp rather than beside the binary, because the plug-in
    folder is often not writable.
*/
inline juce::File cacheDir (const juce::String& product)
{
    return juce::File::getSpecialLocation (juce::File::tempDirectory)
             .getChildFile ("SN manuals")
             .getChildFile (juce::File::createLegalFileName (product));
}

namespace detail
{
    inline bool sameBytes (const juce::File& f, const void* data, int bytes)
    {
        if (! f.existsAsFile() || f.getSize() != (juce::int64) bytes)
            return false;

        juce::MemoryBlock on_disk;
        if (! f.loadFileAsData (on_disk))
            return false;

        return on_disk.getSize() == (size_t) bytes
            && std::memcmp (on_disk.getData(), data, (size_t) bytes) == 0;
    }

    /** Writes one embedded file out. Empty string on success. */
    inline juce::String place (const juce::File& dir, const juce::String& name,
                               const void* data, int bytes, juce::File& out)
    {
        if (data == nullptr || bytes <= 0)
            return "This build has no " + name + " embedded in it.";

        if (auto res = dir.createDirectory(); res.failed())
            return "Could not create " + dir.getFullPathName() + " - " + res.getErrorMessage();

        out = dir.getChildFile (name);

        // Only rewrite when the bytes differ. If a viewer already has this
        // exact file open, rewriting it would either fail outright or make the
        // viewer jump back to page one for no reason.
        if (sameBytes (out, data, bytes))
            return {};

        if (out.replaceWithData (data, (size_t) bytes))
            return {};

        return "Could not write " + out.getFullPathName()
             + " - it may be open in another application, or the folder is read-only.";
    }
}

/** Writes the manual out and hands the page to the operating system.

    The PDF is placed beside the page under the same fixed names the page's
    own link expects. Failing to place the PDF is NOT reported as an error:
    the reader still gets the manual, and the dead link inside the page is
    visible where a dialog about a file they never asked for would only be
    noise. Failing to open the page IS reported - a "?" that does nothing is
    worse than no "?" at all.

    @returns an empty string on success, or the reason the page did not open.
*/
inline juce::String open (const void* htmlData, int htmlBytes,
                          const void* pdfData,  int pdfBytes,
                          const juce::String& product)
{
    auto dir = cacheDir (product);

    juce::File pdf;
    detail::place (dir, "manual.pdf", pdfData, pdfBytes, pdf);

    juce::File page;
    if (auto why = detail::place (dir, "manual.html", htmlData, htmlBytes, page);
        why.isNotEmpty())
        return why;

    if (! page.startAsProcess())
        return "The system would not open " + page.getFullPathName()
             + " - there may be no application registered for .html files.";

    return {};
}

/** Shows why the manual did not open. */
inline void report (juce::Component* owner, const juce::String& why)
{
    juce::ignoreUnused (owner);
    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon,
                                            "Manual", why, "OK");
}

} // namespace manual

/** A round "?" that opens the manual.

    Drawn rather than lettered into the layout so it reads the same at any
    size, and kept deliberately quiet: it is a way out, not a control. The
    colours default to the SnLookAndFeel greys and can be pointed at the
    application's own palette.
*/
class HelpButton : public juce::Button
{
public:
    struct Palette
    {
        juce::Colour ring  { 0xff6b7986 };  ///< the outline
        juce::Colour glyph { 0xffc3cedb };  ///< the question mark
    };

    HelpButton() : juce::Button ("?")
    {
        setTooltip ("Open the manual");
    }

    void setPalette (const Palette& p) { pal = p; repaint(); }
    const Palette& palette() const     { return pal; }

    void paintButton (juce::Graphics& g, bool over, bool down) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (1.0f);
        const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());

        if (d < 6.0f)
            return;   // too small to read; drawing a blob would only mislead

        auto circle = juce::Rectangle<float> (d, d).withCentre (bounds.getCentre());

        if (down || over)
        {
            g.setColour (pal.glyph.withAlpha (down ? 0.28f : 0.13f));
            g.fillEllipse (circle);
        }

        g.setColour (pal.ring);
        g.drawEllipse (circle.reduced (0.75f), 1.2f);

        g.setColour (pal.glyph);
       #if JUCE_MAJOR_VERSION >= 8
        g.setFont (juce::Font (juce::FontOptions (d * 0.60f).withStyle ("Bold")));
       #else
        g.setFont (juce::Font (d * 0.60f, juce::Font::bold));
       #endif
        // Nudged up by a hair: a "?" has no descender, so centring on the glyph
        // box leaves it sitting low inside the circle.
        g.drawText ("?", circle.translated (0.0f, -d * 0.04f),
                    juce::Justification::centred, false);
    }

private:
    Palette pal;
};

} // namespace sn
