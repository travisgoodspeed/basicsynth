//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file SynthWidgetFltk.cpp Platform specific graphics code.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
// Platform-specifc graphics code
// This uses FLTK drawing functions.
// A better implementation should use OpenGL and
// do the linear gradient fill thingy.
// This could also do the cached bitmaps the
// way the windows code does, but with "plain"
// graphics there probably isn't as much
// speed advantage.

#include "globinc.h"
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>

static void SetColor(wdgColor c)
{
	int r = (int) (c >> 16) & 0xff;
	int g = (int) (c >> 8) & 0xff;
	int b = (int) c & 0xff;
	fl_color(r, g, b);
}

static void SetColorMid(wdgColor c1, wdgColor c2)
{
	int r = (int) (((c1 >> 16) & 0xff) + ((c2 >> 16) & 0xff)) / 2;
	int g = (int) (((c1 >> 8) & 0xff) + ((c2 >> 16) & 0xff)) / 2;
	int b = (int) ((c1 & 0xff) + (c2 & 0xff)) / 2;;
	fl_color(r, g, b);
}

void WidgetGroup::Paint(DrawContext dc)
{
	int *offs = (int*)dc;
	SetColor(bgClr);
	fl_rectf(area.x+offs[0], area.y+offs[1], area.w, area.h);
	if (border)
	{
		SetColor(frClr);
		fl_rect(area.x+offs[0], area.y+offs[1], area.w, area.h);
	}
	if (show)
		EnumList(EnumPaint, dc);
}

void KnobData::CreateImage() { }
void KnobData::DestroyImage() { }

void KnobWidget::CreateImage()
{
}

void KnobWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;

	int *offs = (int *)dc;
	int dx = offs[0];
	int dy = offs[1];
	wdgRect rc;
	if (enable)
	{
		rc = area;
		rc.Offset(dx, dy);
		SetColor(bgClr);
		fl_rectf(rc.x, rc.y, rc.w, rc.h);
		SetColorMid(highClr, faceClr);
		rc = dial;
		rc.Offset(dx, dy);
		fl_pie(rc.x, rc.y, rc.w-1, rc.h-1, 0.0, 360.0);
		fl_color(0,0,0);
		fl_arc(rc.x, rc.y, rc.w, rc.h, 0.0, 360.0);
		fl_line(centerPt.x+dx, centerPt.y+dy, valuePt.x+dx, valuePt.y+dy);
	}
	else
	{
		rc = dial;
		rc.Offset(dx, dy);
		SetColor(bgClr);
		fl_pie(rc.x, rc.y, rc.w-1, rc.h-1, 0.0, 360.0);
		fl_color(0,0,0);
		fl_arc(rc.x, rc.y, rc.w, rc.h, 0.0, 360.0);
	}
}

void KnobWidget::DrawTicks(DrawContext dc, wdgRect& bk, wdgRect& fc)
{
}

void KnobBlack::CreateImage()
{
}

void KnobBlack::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	if (!enable)
	{
		KnobWidget::Paint(dc);
		return;
	}

	int *offs = (int*)dc;
	int dx = offs[0];
	int dy = offs[1];

	SetColor(bgClr);
	wdgRect rc = area;
	rc.Offset(dx, dy);
	fl_rectf(rc.x, rc.y, rc.w, rc.h);
	fl_color(0x40,0x40,0x40);
	rc = shaft;
	rc.Offset(dx, dy);
	fl_pie(rc.x, rc.y, rc.w-1, rc.h-1, 0.0, 360.0);
	fl_arc(rc.x, rc.y, rc.w, rc.h, 0.0, 360.0);
	SetColor(faceClr);
	rc = dial;
	rc.Offset(dx, dy);
	fl_pie(rc.x, rc.y, rc.w-1, rc.h-1, 0.0, 360.0);
	fl_arc(rc.x, rc.y, rc.w, rc.h, 0.0, 360.0);
	fl_color(0,0,0);
	fl_line(centerPt.x+dx, centerPt.y+dy, valuePt.x+dx, valuePt.y+dy);
}

void KnobBased::CreateImage()
{
}

void GradientFillRect(wdgRect& rc, wdgColor loClr, wdgColor hiClr)
{
	double cy = (double) rc.Height();
	double red1 = (double) ((loClr >> 16) & 0xff);
	double grn1 = (double) ((loClr >> 8) & 0xff);
	double blu1 = (double) (loClr & 0xff);
	double red2 = (double) ((hiClr >> 16) & 0xff);
	double grn2 = (double) ((hiClr >> 8) & 0xff);
	double blu2 = (double) (hiClr & 0xff);
	double rinc = (red2 - red1) / cy;
	double ginc = (grn2 - grn1) / cy;
	double binc = (blu2 - blu1) / cy;
	int y = rc.GetTop();
	while (y <= rc.GetBottom())
	{
		fl_color((int)red2, (int)grn2, (int)blu2);
		fl_line(rc.GetLeft(), y, rc.GetRight(), y);
		red2 -= rinc;
		blu2 -= binc;
		grn2 -= ginc;
		y++;
	}
}

// experimental - needs work.
void GradientFillCircle(wdgRect& rc, wdgColor loClr, wdgColor hiClr)
{
	double rx = (double) (rc.w / 2);
	double ry = (double) (rc.h / 2);
	double cx = (double) rc.x + rx;
	double cy = (double) rc.y + ry;

	double red1 = (double) ((loClr >> 16) & 0xff);
	double grn1 = (double) ((loClr >> 8) & 0xff);
	double blu1 = (double) (loClr & 0xff);
	double red2 = (double) ((hiClr >> 16) & 0xff);
	double grn2 = (double) ((hiClr >> 8) & 0xff);
	double blu2 = (double) (hiClr & 0xff);
	double rinc = (red2 - red1) / rx;
	double ginc = (grn2 - grn1) / rx;
	double binc = (blu2 - blu1) / rx;

	double r2 = rx * rx;
	double x = rx;
	double y = 0;
	while (y <= ry)
	{
		x = sqrt(r2 - (y * y));
		fl_color((int)red2, (int)grn2, (int)blu2);
		fl_line((int)(cx - x)+1, (int)(cy - y), (int)(cx + x)-1, (int)(cy - y));
		fl_line((int)(cx - x)+1, (int)(cy + y), (int)(cx + x)-1, (int)(cy + y));
		y += 1.0;
		red2 -= rinc;
		blu2 -= binc;
		grn2 -= ginc;
	}
/*	x = rx;
	y = 0;
	int pts = 0;
	double d1 = 0;
	double d2 = 0;
	while (x > y)
	{
		y += 1.0;
		d1 = sqrt(r2 - (y * y));
		d1 = ceil(d1) - d1;
		if (d1 < d2)
			x -= 1.0;
		d2 = d1;
		pts++;
	}

	printf("Circle of %d radius has %d points\n", (int) rx, pts);*/
}

void KnobBased::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	if (!enable)
	{
		KnobWidget::Paint(dc);
		return;
	}

	int *offs = (int*)dc;
	int dx = offs[0];
	int dy = offs[1];
	wdgRect rca = area;
	rca.Offset(dx, dy);
	wdgRect rcb = base;
	rcb.Offset(dx, dy);
	wdgRect rcs = shaft;
	rcs.Offset(dx, dy);
	wdgRect rcd = dial;
	rcd.Offset(dx, dy);

	fl_line_style(FL_SOLID, 0);
	SetColor(bgClr);
	fl_rectf(rca.x, rca.y, rca.w, rca.h);

	fl_color(150,150,150);
	fl_pie(rcb.x, rcb.y, rcb.w, rcb.h, 0.0, 360.0);
//	GradientFillCircle(rcb, 0xffa0a0a0, 0xfff0f0f0);

	fl_color(40,40,40);
	fl_pie(rcs.x, rcs.y, rcs.w, rcs.h, 0.0, 360.0);
//	GradientFillCircle(rcs, 0xff808080, 0xff404040);

//	SetColorMid(faceClr, highClr);
	SetColor(faceClr);
	fl_pie(rcd.x, rcd.y, rcd.w, rcd.h, 0.0, 360.0);
//	GradientFillCircle(rcd, faceClr, highClr);

	fl_color(0,0,0);
	fl_arc(rcb.x, rcb.y, rcb.w, rcb.h, 0.0, 360.0);
//	fl_arc(rcs.x, rcs.y, rcs.w, rcs.h, 0.0, 360.0);
//	fl_arc(rcd.x, rcd.y, rcd.w, rcd.h, 0.0, 360.0);

	fl_line_style(FL_SOLID, 3);
	fl_line(centerPt.x+dx, centerPt.y+dy, valuePt.x+dx, valuePt.y+dy);
}

void SwitchData::CreateImage() { }
void SwitchData::DestroyImage() { }

void SwitchWidget::CreateImage()
{
}

void SwitchWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	int *offs = (int*)dc;
	int dx = offs[0];
	int dy = offs[1];
	wdgRect rca = area;
	rca.Offset(dx, dy);
	wdgRect swrc = rca;
	swrc.Shrink(1, 1);

	if (!show || !enable)
	{
		SetColor(bgClr);
		fl_rectf(rca.x, rca.y, rca.w, rca.h);
	}
	else if (swOn)
		GradientFillRect(rca, swdnlo, swdnhi);
	else
		GradientFillRect(rca, swuplo, swuphi);

/*
	if (!show || !enable)
		SetColor(bgClr);
	else if (swOn)
		fl_color(90,90,90);
	else
		fl_color(132,132,132);
	fl_rectf(rca.x, rca.y, rca.w, rca.h);
*/

	if (show)
	{
		fl_color(0,0,0);
		fl_rect(rca.x, rca.y, rca.w, rca.h);
	}

	if (enable)
	{
		fl_line_style(FL_SOLID, 2);
		if (swOn)
			fl_color(220,220,180);
		else
			fl_color(24,24,24);
		fl_line(swrc.x, swrc.y + swrc.h, swrc.x + swrc.w, swrc.y + swrc.h);
		fl_line(swrc.x + swrc.w, swrc.y, swrc.x + swrc.w, swrc.y + swrc.h);

		if (!swOn)
			fl_color(220,220,180);
		else
			fl_color(24,24,24);
		fl_line(swrc.x, swrc.y, swrc.x + swrc.w, swrc.y);
		fl_line(swrc.x, swrc.y, swrc.x, swrc.y + swrc.h);
	}
	DrawLabel(dc);
}

void SwitchWidget::DrawLabel(DrawContext dc)
{
	if (lbl && show)
	{
		int *offs = (int*)dc;
		int dx = offs[0];
		int dy = offs[1];
		wdgRect rca = area;
		rca.Offset(dx, dy);
		int curfont = fl_font();
		int cursize = fl_size();
		int newfont = FL_HELVETICA;
		if (bold)
			newfont |= FL_BOLD;
		if (italic)
			newfont |= FL_ITALIC;
		fl_font(newfont, lblHeight);
		if (enable)
			SetColor(fgClr);
		else
			//BlendColor(bgClr, 0x80000000);
			fl_color(0,0,0);
		fl_draw(lbl, rca.x, rca.y, rca.w, rca.h, FL_ALIGN_CENTER);
		fl_font(curfont, cursize);
	}
}

void SlideSwitchWidget::CreateImage()
{
}

void SlideSwitchWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
}

void *ImageWidget::LoadImage(char *file)
{
	char path[MAX_PATH];
	if (SynthFileExists(file))
		strncpy(path, file, MAX_PATH);
	else
		snprintf(path, MAX_PATH, "%s/%s", form->FormsDir(), file);
	if (strstr(file, ".jpg"))
		return (void*) new Fl_JPEG_Image(path);
	if (strstr(file, ".png"))
		return (void*) new Fl_PNG_Image(path);
	if (strstr(file, ".bmp"))
		return (void*) new Fl_BMP_Image(path);
	return 0;
}

void ImageWidget::CreateImage()
{
	sipImg = new ImageCacheItem;
	sipImg->AddRef();
	SwitchData *sd = new SwitchData;
	sipImg->SetData(sd);
	sd->type = nullSwitch;
	sd->cx = area.w;
	sd->cy = area.h;
	sd->bm[0] = LoadImage(offImg);
	sd->bm[1] = LoadImage(onImg);
}

void ImageWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	SwitchWidget::Paint(dc);

	int *offs = (int*)dc;

	if (!sipImg)
		CreateImage();

	Fl_RGB_Image *img = (Fl_RGB_Image*)(sipImg->GetImage(swOn ? 1 : 0));
	if (img)
	{
		int iw = img->w();
		int ih = img->h();
		int ix = area.x + (area.w - iw)/2;
		int iy = area.y + (area.h - ih)/2;
		img->draw(ix+offs[0], iy+offs[1]);
	}
}

void LampWidget::CreateImage()
{
}

void LampWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	int *offs = (int*)dc;
	int dx = offs[0];
	int dy = offs[1];
	wdgRect rca = area;
	rca.Offset(dx, dy);

	fl_line_style(FL_SOLID, 0);
	SetColor(bgClr);
	fl_rectf(rca.x, rca.y, rca.w, rca.h);

	if (enable)
	{
		wdgRect rcb;
		rcb = rca;
		if (!(style & 2))
		{
			if (rca.w > rca.h)
				rcb.w = rca.h;
			else
				rcb.h = rca.w;
		}

		fl_color(150,150,150);
		if (style == 0)
			fl_pie(rcb.x, rcb.y, rcb.w, rcb.h, 0.0, 360.0);
		else
			fl_rectf(rcb.x, rcb.y, rcb.w, rcb.h);
		fl_color(0,0,0);
		if (style == 0)
			fl_arc(rcb.x, rcb.y, rcb.w, rcb.h, 0.0, 360.0);
		else
			fl_rect(rcb.x, rcb.y, rcb.w, rcb.h);

		wdgRect rcl;
		rcl = rcb;
		rcl.Shrink(4, 4);

		if (swOn)
			SetColor(loColor);
		else
			fl_color(40,40,40);
		if (style == 0)
			fl_pie(rcl.x, rcl.y, rcl.w, rcl.h, 0.0, 360.0);
		else
			fl_rectf(rcl.x, rcl.y, rcl.w, rcl.h);
	}
	else if (show)
	{
		fl_color(0,0,0);
		fl_rect(rca.x, rca.y, rca.w, rca.h);
	}

//	DrawLabel(dc);
	if (lbl && show)
	{
		int curfont = fl_font();
		int cursize = fl_size();
		int newfont = FL_HELVETICA;
		if (bold)
			newfont |= FL_BOLD;
		if (italic)
			newfont |= FL_ITALIC;
		fl_font(newfont, lblHeight);
		if (enable)
			SetColor(fgClr);
		else
			fl_color(0,0,0);
		fl_draw(lbl, lblRect.x, lblRect.y, lblRect.w, lblRect.h, (style & 2) ? FL_ALIGN_CENTER : FL_ALIGN_LEFT);
		fl_font(curfont, cursize);
	}
}

void BoxWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	if (!show)
		return;
	int *offs = (int*)dc;
	wdgRect rca = area;
	rca.Offset(offs[0], offs[1]);

	fl_line_style(FL_SOLID, (int)thick);

	SetColor(bgClr);
	fl_rectf(rca.x, rca.y, rca.w, rca.h);

	SetColor(fgClr);
	fl_rect(rca.x, rca.y, rca.w, rca.h);
}

void TextWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	if (show)
	{
		int *offs = (int*)dc;
		wdgRect rca = area;
		rca.Offset(offs[0], offs[1]);

		if (filled)
		{
			SetColor(bgClr);
			fl_rectf(rca.x, rca.y, rca.w, rca.h);
		}
		Fl_Align flal;
		if (align == 0)
			flal = FL_ALIGN_LEFT;
		else if (align == 1)
			flal = FL_ALIGN_CENTER;
		else
			flal = FL_ALIGN_RIGHT;
		int curfont = fl_font();
		int cursize = fl_size();
		int newfont = FL_HELVETICA;
		if (textBold)
			newfont |= FL_BOLD;
		if (textItalic)
			newfont |= FL_ITALIC;
		fl_font(newfont, textHeight);
		if (inset)
		{
			fl_color(0,0,0);
			fl_draw(text, rca.x-inset, rca.y-inset, rca.w, rca.h, flal);
			fl_color(255,255,255);
			fl_draw(text, rca.x+inset, rca.y+inset, rca.w, rca.h, flal);
		}
		else if (shadow)
		{
			fl_color(0,0,0);
			fl_draw(text, rca.x+shadow, rca.y+shadow, rca.w, rca.h, flal);
		}
		SetColor(fgClr);
		fl_draw(text, rca.x, rca.y, rca.w, rca.h, flal);
		fl_font(curfont, cursize);
	}
}

void GraphWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	if (!show)
		return;

	int *offs = (int*)dc;
	wdgRect rca = area;
	rca.Offset(offs[0], offs[1]);

	SetColor(bgClr);
	fl_rectf(rca.x, rca.y, rca.w, rca.h);
	SetColor(fgClr);
	fl_rect(rca.x, rca.y, rca.w, rca.h);

	if (numVals < 1 || vals == 0)
		return;

	SetColor(grClr);
	float scale = (float) (rca.h - 2) / range;
	float incr = (float) (rca.w - 2) / (float) numVals;
	if (bar)
	{
		wdgRect r;
		float x = (float) rca.x + 1;
		r.w = (int) (incr * barWidth);
		for (int n = 0; n < numVals; n++)
		{
			float y = vals[n] * scale;
			r.x = (int)x;
			r.y = rca.GetBottom() - 1 - (int)y;
			r.h = (int)y;
			fl_rectf(r.x, r.y, r.w, r.h);
			x += incr;
		}
	}
	else
	{
		fl_line_style(FL_SOLID, (int)thick);
		float x, y;
		int x1, y1, x2, y2;
		x1 = rca.x + 1;
		y1 = rca.GetBottom();
		x = (float) x1;
		for (int n = 0; n < numVals; n++)
		{
			y = vals[n] * scale;
			x2 = (int) x;
			y2 = rca.GetBottom() - 1 - (int)y;
			fl_line(x1, y1, x2, y2);
			x1 = x2;
			y1 = y2;
			x += incr;
		}
	}
}

void EnvelopeWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	int *offs = (int*)dc;
	wdgRect rca = area;
	rca.Offset(offs[0], offs[1]);

	SetColor(bgClr);
	fl_rectf(rca.x, rca.y, rca.w, rca.h);
	if (border)
	{
		SetColor(frClr);
		fl_rect(rca.x, rca.y, rca.w, rca.h);
	}
	if (numSegs < 1 || vals == 0)
		return;
	int num = numSegs;

	double lvlscale = (double) (rca.h - 2) / ampRange;
	double rtscale = (double) (rca.w - 2) / tmRange;

	double rtval = 0;
	double lvlval = 0;
	double xo = (double) (rca.x + 1);
	double yo = (double) (rca.y + rca.h - 1);
	double lasty = 0;
	double lastx = 0;
	SetColor(fgClr);
	fl_begin_line();
	fl_vertex(xo, yo);
	if (start != 0)
	{
		lastx = xo;
		lasty = yo - (start * lvlscale);
		fl_vertex(lastx, lasty);
	}
	if (susOn)
		num--;
	int ndx;
	for (ndx = 0; ndx < num; ndx++)
	{
		rtval += vals[ndx].rate;
		if (rtval > tmRange)
			rtval = tmRange;
		lvlval = vals[ndx].level;
		if (lvlval > ampRange)
			lvlval = ampRange;
		lastx = xo + (rtval * rtscale);
		lasty = yo - (lvlval * lvlscale);
		fl_vertex(lastx, lasty);
	}
	if (susOn)
	{
		double x1 = (double) rca.GetRight() - 1;
		double x2 = x1 - (vals[ndx].rate * rtscale);
		if (x2 > lastx)
		{
			fl_vertex(x2, lasty);
			lastx = x1;
		}
		lvlval = vals[ndx].level;
		if (lvlval > ampRange)
			lvlval = ampRange;
		lasty = yo - (lvlval * lvlscale);
		fl_vertex(lastx, lasty);
	}
	else if (lasty != yo)
		fl_vertex(lastx, yo);
	fl_end_line();
}

void WaveWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	int *offs = (int*)dc;
	wdgRect rca = area;
	rca.Offset(offs[0], offs[1]);

	SetColor(bgClr);
	fl_rectf(rca.x, rca.y, rca.w, rca.h);
	if (border)
	{
		SetColor(frClr);
		fl_rect(rca.x, rca.y, rca.w, rca.h);
	}

	if (wvType == 1)
		PlotSum(dc);
	else if (wvType == 2)
		PlotSeg(dc);
}

void WaveWidget::PlotSum(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	int *offs = (int*)dc;
	wdgRect rca = area;
	rca.Offset(offs[0], offs[1]);
	int cx = area.w - 4;
	int cy = area.h - 4;

	if (cx > 0 && cy > 0)
	{
		int n;
		int parts = 0;
		int x;
		double mid = (double)cy / 2.0;
		double maxmul = 1;
		double pp[WFI_MAXPART];
		for (n = 0; n < WFI_MAXPART; n++)
		{
			pp[n] = phs[n];
			if (on[n])
			{
				parts++;
				if (n > maxmul)
					maxmul = n;
			}
		}

		double phsIncr = twoPI / cx;
		double *ampvals = new double[cx];
		double maxAmp = 0.0;
		double sigma;
		double sigK = PI / maxmul;
		double sigN;
		for (x = 0; x < cx; x++)
		{
			ampvals[x] = 0.0;
			for (n = 0; n < WFI_MAXPART; n++)
			{
				if (!on[n])
					continue;
				if (gibbs && n > 0)
				{
					sigN = (double)n * sigK;
					sigma = amps[n] * (sin(sigN) / sigN);
				}
				else
					sigma = amps[n];
				ampvals[x] += sin(pp[n]) * sigma;
				pp[n] += phsIncr * (double) (n+1);
				if (pp[n] >= twoPI)
					pp[n] -= twoPI;
			}
			if (fabs(ampvals[x]) > maxAmp)
				maxAmp = fabs(ampvals[x]);
		}
		SetColor(fgClr);
		fl_line(rca.x + 2, rca.y + 2 + (int) mid, rca.GetRight() - 2, rca.y + 2 + (int) mid);
		fl_begin_line();
		double xo = (double) (area.x + 2);
		double yo = (double) (area.y + 2);
		for (x = 0; x < cx; x++)
		{
			fl_vertex(xo + (double)x, yo + (mid - ((ampvals[x] / maxAmp) * mid)));
		}

		fl_end_line();
		delete ampvals;
	}
}

void WaveWidget::PlotSeg(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	int *offs = (int*)dc;
	wdgRect rca = area;
	rca.Offset(offs[0], offs[1]);

	int cx = area.w - 4;
	int cy = area.h - 4;

	if (cx > 0 && cy > 0)
	{
		SetColor(fgClr);
		int n;
		double mid = (double) cy / 2.0;
		double xo = (double) (rca.x + 2);
		double yo = (double) (rca.y + 2);
		double x;
		double y;
		fl_line(rca.x + 2, rca.y + 2 + (int) mid, rca.GetRight() - 2, rca.y + 2 + (int) mid);
		fl_begin_line();
		fl_vertex(area.x, area.y + mid);
		x = 0;
		for (n = 0; n < WFI_MAXPART; n++)
		{
			if (!on[n])
				continue;
			x += (phs[n] * cx);
			if (x > cx)
				x = cx;
			y = (mid - (amps[n] * mid));
			if (y < 0)
				y = 0;
			else if (y > (double)cy)
				y = (double)cy;
			fl_vertex(xo + x, yo + y);
		}
		fl_end_line();
	}
}

void SliderWidget::CreateImage()
{
}

void SliderWidget::Paint(DrawContext dc)
{
	if (!fl_not_clipped(area.x, area.y, area.w, area.h))
		return;
	if (!show)
		return;
	int *offs = (int*)dc;
	wdgRect rca = area;
	rca.Offset(offs[0], offs[1]);

	fl_line_style(FL_SOLID, 1);

	fl_color(128,128,128);
	fl_rectf(rca.x, rca.y, rca.w, rca.h);

	fl_color(40,40,40);
	fl_rect(rca.x, rca.y, rca.w, rca.h);
	fl_color(220,220,220);
	fl_line(rca.x, rca.GetBottom(), rca.x, rca.y, rca.GetRight(), rca.y);

	fl_line_style(FL_SOLID, 3);
	fl_color(10, 10, 10);
	if (vertical)
		fl_line(slideTrack.x, slideTrack.y, slideTrack.x, slideTrack.GetBottom());
	else
		fl_line(slideTrack.x, slideTrack.y, slideTrack.GetRight(), slideTrack.y);

	fl_color(32,32,32);
	fl_rectf(knobPos.x, knobPos.y, knobPos.w, knobPos.h);

	fl_color(220,220,220);
	if (vertical)
	{
		int half = knobPos.y + (knobPos.h/2);
		fl_line(knobPos.x, half, knobPos.GetRight(), half);
	}
	else
	{
		int half = knobPos.x + (knobPos.w/2);
		fl_line(half, knobPos.y, half, knobPos.GetBottom());
	}
}
