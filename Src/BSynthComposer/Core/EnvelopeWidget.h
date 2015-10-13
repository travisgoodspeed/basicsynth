//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _ENVWIDGET_H_
#define _ENVWIDGET_H_

/// Widget to display an envelope graph.
/// The number of segments and range of each segment
/// are set at runtime. If the susOn flag is set,
/// the envelope is extended to fill the entire display
/// area with the last segment treated as the release.
/// Otherwise, the envelope graphi is drawn without
/// a sustain portion and ends at the total time
/// for the segments. When setting the time range, the
/// value should be longer than the total time of the
/// segments. Likewise, the amplitude range should be
/// set to the maximum amplitude value of the envelope
/// or greater.
class EnvelopeWidget : public SynthWidget
{
private:
	int numSegs;
	SegVals *vals;
	float start;
	int susOn;
	int border;
	float tmRange;
	float ampRange;
	wdgColor frClr;

public:
	EnvelopeWidget();
	~EnvelopeWidget();

	/// Set the time range.
	/// @param t total time to display (in seconds)
	void SetTime(float t)
	{
		tmRange = t;
	}

	/// Set the amplitude level range.
	/// @param a maximum amplitude
	void SetLevel(float a)
	{
		ampRange = a;
	}

	/// Set the start value.
	/// @param st starting level
	void SetStart(float st)
	{
		start = st;
	}

	/// Set the sustain flag.
	/// @param on sustain on when non-zero
	void SetSus(int on)
	{
		susOn = on;
	}

	/// Set the number of segments
	/// @param n number of segments
	void SetSegs(int n);
	/// Set values for one segment.
	/// @param n segment number (zero-based)
	/// @param rt rate in seconds
	/// @param lvl amplitude level at end of segment
	void SetVal(int n, float rt, float lvl);

	virtual int Load(XmlSynthElem *elem);
	/// Redraw the widget.
	virtual void Paint(DrawContext dc);
};

#endif
