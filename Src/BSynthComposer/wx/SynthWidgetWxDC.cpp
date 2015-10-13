//////////////////////////////////////////////////////////////////////
// Platform-specifc graphics code for wxWidgets
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#if !wxUSE_GRAPHICS_CONTEXT

// Wx colors have RED in the LSB...
inline long SwapRGB(long c)
{
	return ((c>>16)&0x0000ff)|(c&0x00ff00)|((c<<16)&0xff0000);
}

inline wxColour ColorRGB(long c)
{
	return wxColour((c>>16)&0xff,(c>>8)&0xff,c&0xff);
}

void WidgetGroup::Paint(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;
	wxBrush bck(ColorRGB(bgClr));
	gr->SetBrush(bck);
	if (border)
		gr->SetPen(wxPen(ColorRGB(frClr), (int)border));
	else
		gr->SetPen(wxPen(ColorRGB(bgClr), 0));
	gr->DrawRectangle(area.x, area.y, area.w, area.h);
	if (style != 0)
	{
		gr->SetPen(wxPen(ColorRGB(fgClr), (int)border));
		if (style == 1) // inset
		{
			if (area.w > 1)
				gr->DrawLine(area.GetLeft(), area.GetBottom(), area.GetRight(), area.GetBottom());
			if (area.h > 1)
				gr->DrawLine(area.GetRight(), area.GetTop(), area.GetRight(), area.GetBottom());
		}
		else // outset
		{
			if (area.w > 1)
				gr->DrawLine(area.GetLeft(), area.GetTop(), area.GetRight(), area.GetTop());
			if (area.h > 1)
				gr->DrawLine(area.GetLeft(), area.GetTop(), area.GetLeft(), area.GetBottom());
		}
	}

	if (show)
		EnumList(EnumPaint, dc);
}

// Create/Destroy image functions can be used to
// implement a bitmap cache for Knobs and switches.
// A bitmap cache is created by drawing the item once,
// saving it, then bitblt during refresh. Most graphics
// engines are fast enough that this isn't all that useful
// anymore. See the mswin implementation for an example.
void SwitchData::DestroyImage()
{
	delete (wxBitmap*)bm[0];
	delete (wxBitmap*)bm[1];
	bm[0] = 0;
	bm[1] = 0;
}

void SwitchData::CreateImage()
{
	DestroyImage();
	wxBitmap *tmp = new wxBitmap(cx, cy);
	bm[0] = (void *)tmp;
	tmp = new wxBitmap(cx, cy);
	bm[1] = (void *)tmp;
}

void KnobData::CreateImage()
{
	DestroyImage();
	wxBitmap *tmp = new wxBitmap(cx, cy);
	bm = (void*)tmp;
}

void KnobData::DestroyImage() {}

void KnobWidget::CreateImage() { }

void KnobWidget::Paint(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;
	gr->SetBrush(wxBrush(ColorRGB(bgClr)));
	gr->SetPen(wxPen(ColorRGB(bgClr)));
	gr->DrawRectangle(area.GetLeft(), area.GetTop(), area.Width(), area.Height());
	if (enable)
	{
		gr->SetPen(wxPen(ColorRGB(fgClr), 2));
		gr->SetBrush(wxBrush(ColorRGB(faceClr)));
		gr->DrawEllipse(dial.GetLeft(), dial.GetTop(), dial.Width(), dial.Height());
		gr->DrawLine(centerPt.x, centerPt.y, valuePt.x, valuePt.y);
	}
	else
	{
		gr->SetPen(wxPen(wxColour(0,0,0,0x80)));
		gr->SetBrush(wxBrush(*wxBLACK,wxTRANSPARENT));
		gr->DrawEllipse(dial.GetLeft(), dial.GetTop(), dial.Width(), dial.Height());
	}
//	if (focus)
//	{
//		wxPen olPen(ColorRGB(fgClr));
//		gr->SetPen(olPen);
//		gr->DrawRectangle(&olPen, area.x+1, area.y+1, area.w-2, area.h-2);
//	}
}

void KnobWidget::DrawTicks(DrawContext dc, wdgRect& bk, wdgRect& fc)
{
	wxDC *gr = (wxDC*)dc;

	if (tickMaj > 0 && tickLen > 0)
	{
		gr->SetPen(wxPen(ColorRGB(fgClr), 1));
		wxDouble hw = (wxDouble) bk.w / 2.0;
		wxDouble hh = (wxDouble) bk.h/ 2.0;
		wxDouble xc = fc.x + ((wxDouble)fc.w / 2.0);
		wxDouble yc = fc.y + ((wxDouble)fc.h / 2.0);
		wxDouble rx1 = hw;
		wxDouble rx2 = rx1 - (tickLen/2);
		wxDouble ry1 = hh;
		wxDouble ry2 = ry1 - (tickLen/2);
		wxDouble inc1 = pi150 / (wxDouble) tickMaj;
		wxDouble inc2 = inc1;
		if (tickMin > 0)
			inc2 /= (wxDouble) tickMin;
		wxDouble x1, y1;
		wxDouble ang = pi125;
		for (int n = 0; n < tickMaj; n++)
		{
			ang = pi125 - (n * inc1);
			x1 = xc + (cos(ang) * rx1);
			y1 = yc - (sin(ang) * ry1);
			gr->DrawLine(x1, y1, xc, yc);
			if (tickMin > 0)
			{
				wxDouble ang2 = ang;
				for (int m = 0; m < tickMin; m++)
				{
					x1 = xc + (cos(ang2) * rx2);
					y1 = yc - (sin(ang2) * ry2);
					gr->DrawLine(x1, y1, xc, yc);
					ang2 -= inc2;
				}
			}
			ang -= inc1;
		}
		//ang = pi125 - pi150;
		x1 = xc + (cos(ang) * rx1);
		y1 = yc - (sin(ang) * ry1);
		gr->DrawLine(x1, y1, xc, yc);
	}
}

void KnobBlack::CreateImage() {}

void KnobBlack::Paint(DrawContext dc)
{
	if (!enable)
	{
		KnobWidget::Paint(dc);
		return;
	}

	wxDC *gr = (wxDC*)dc;

	wxPen indPen(ColorRGB(fgClr), 2);
	wxPen olPen(wxColour(0,0,0), 4);
	wxBrush backBrush(ColorRGB(bgClr));
	wxBrush baseBrush(wxColour(0,0,0),wxVERTICAL_HATCH);
	wxBrush topBrush(ColorRGB(faceClr));
	gr->SetPen(wxPen(ColorRGB(bgClr)));
	gr->SetBrush(backBrush);
	gr->DrawRectangle(area.x, area.y, area.w, area.h);
	gr->SetPen(olPen);
	gr->SetBrush(baseBrush);
	gr->DrawEllipse(shaft.x, shaft.y, shaft.w, shaft.h);
	gr->SetBrush(topBrush);
	gr->DrawEllipse(dial.x, dial.y, dial.w, dial.h);

	gr->SetPen(indPen);
	gr->DrawLine(centerPt.x, centerPt.y, valuePt.x, valuePt.y);
}

void KnobBased::CreateImage() {}

void KnobBased::Paint(DrawContext dc)
{
	if (!enable)
	{
		KnobWidget::Paint(dc);
		return;
	}

	wxDC *gr = (wxDC*)dc;


	wxBrush backBrush(ColorRGB(bgClr));
	gr->SetBrush(backBrush);
	gr->SetPen(wxPen(ColorRGB(bgClr)));
	gr->DrawRectangle(area.x, area.y, area.w, area.h);

	wxBrush baseBrush(wxColour(64,64,64));
	wxBrush midBrush(wxColour(0,0,0));
	wxBrush topBrush(ColorRGB(faceClr));

	wxPen olPen(wxColour(10,10,10),0);
	gr->SetPen(olPen);
	gr->SetBrush(baseBrush);
	gr->DrawEllipse(base.x, base.y, base.w, base.h);
	gr->SetBrush(midBrush);
	gr->DrawEllipse(shaft.x, shaft.y, shaft.w, shaft.h);
	gr->SetBrush(topBrush);
	gr->DrawEllipse(dial.x, dial.y, dial.w, dial.h);

	wxPen indPen(fgClr, 2);
	gr->SetPen(indPen);
	gr->DrawLine(centerPt.x, centerPt.y, valuePt.x, valuePt.y);
}

void SwitchWidget::CreateImage() {}

void SwitchWidget::Paint(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;

	wxBrush bck(ColorRGB(bgClr));
	wxPen pnBlk(wxColour(24,24,24));
	gr->SetBrush(bck);
	if (!show || !enable)
	{
		if (!show)
			pnBlk.SetStyle(wxTRANSPARENT);
		gr->SetPen(pnBlk);
		gr->SetBrush(bck);
		gr->DrawRectangle(area.x, area.y, area.w, area.h);
	}
	else
	{
		wxPen pnWht(wxColour(220,220,180));
		wdgRect swrc;
		swrc = area;
		swrc.Shrink(1,1);

		gr->SetPen(pnBlk);
		if (swOn)
		{
			gr->GradientFillLinear(wxRect(area.x, area.y, area.w, area.h), ColorRGB(swdnlo), ColorRGB(swdnhi));
			gr->DrawLine(swrc.x, swrc.y, swrc.x + swrc.w, swrc.y);
			gr->DrawLine(swrc.x, swrc.y, swrc.x, swrc.y + swrc.h);
			gr->SetPen(pnWht);
			gr->DrawLine(swrc.x, swrc.y + swrc.h, swrc.x + swrc.w, swrc.y + swrc.h);
			gr->DrawLine(swrc.x + swrc.w, swrc.y, swrc.x + swrc.w, swrc.y + swrc.h);
		}
		else
		{
			gr->GradientFillLinear(wxRect(area.x, area.y, area.w, area.h), ColorRGB(swuphi), ColorRGB(swuplo));
			gr->DrawLine(swrc.x, swrc.y + swrc.h, swrc.x + swrc.w, swrc.y + swrc.h);
			gr->DrawLine(swrc.x + swrc.w, swrc.y, swrc.x + swrc.w, swrc.y + swrc.h);
			gr->SetPen(pnWht);
			gr->DrawLine(swrc.x, swrc.y, swrc.x + swrc.w, swrc.y);
			gr->DrawLine(swrc.x, swrc.y, swrc.x, swrc.y + swrc.h);
		}
	}
	DrawLabel(dc);
}


void SwitchWidget::DrawLabel(DrawContext dc)
{
	if (lbl && show)
	{
		wxDC *gr = (wxDC*)dc;

		int th = (72 * lblHeight) / 96;
		wxFont font(th, wxFONTFAMILY_SWISS,
			italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);

		gr->SetFont(font);
		gr->SetTextForeground(wxColour(ColorRGB(fgClr)));
		wxString grText((const char*)lbl);
		wxCoord width, height, descent, externalLeading;
		gr->GetTextExtent(grText, &width, &height, &descent, &externalLeading);

		wxDouble tx;
		wxDouble ty;
		if (align == 0)
			tx = (wxDouble)lblRect.x;
		else if (align == 1)
			tx = (wxDouble)lblRect.x + ((area.w - width) / 2.0);
		else
			tx = (wxDouble)(lblRect.x + lblRect.w) - width;
		ty = (wxDouble)lblRect.y + ((lblRect.h - height) / 2.0);
		if (shadow)
		{
			gr->SetFont(font);
			gr->SetTextForeground(wxColour(0,0,0));
			gr->DrawText(grText, tx+shadow, ty+shadow);
		}
		gr->SetFont(font);
		if (enable)
			gr->SetTextForeground(ColorRGB(fgClr));
		else
			gr->SetTextForeground(wxColour(0,0,0,0x80));
		gr->DrawText(grText, tx, ty);
	}
}

void SlideSwitchWidget::CreateImage() {}

void SlideSwitchWidget::Paint(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;

	wxRect bgrc;
	wxRect inrc;
	wxRect swrc;
	bgrc.SetLeft(area.x);
	bgrc.SetTop(area.y);
	bgrc.SetWidth(area.w);
	bgrc.SetHeight(area.h);
	inrc = bgrc;
	inrc.Inflate(-2, -2);
	swrc = inrc;
	swrc.Inflate(-2, -2);
	if (style == 0)
	{	// horizontal
		swrc.SetWidth(swrc.GetWidth() / 2);
		if (swOn)
			swrc.Offset(swrc.GetWidth(), 0);
	}
	else
	{
		// vertical
		swrc.SetHeight(swrc.GetHeight() / 2);
		if (swOn)
			swrc.Offset(0, swrc.GetHeight());
	}

	wxPen pnBlk(wxColor(24,24,24), 2);
	wxPen pnWht(wxColor(180,176,170), 2);
	pnBlk.SetCap(wxCAP_PROJECTING);
	pnWht.SetCap(wxCAP_PROJECTING);

	gr->GradientFillLinear(bgrc, wxColour(50,50,50), wxColour(128,128,128));
	gr->GradientFillLinear(swrc, wxColour(148,148,148), wxColour(64,64,64));

	gr->SetPen(pnBlk);
	gr->DrawLine(inrc.GetLeft(), inrc.GetTop(), inrc.GetRight(), inrc.GetTop());
	gr->DrawLine(inrc.GetLeft(), inrc.GetTop(), inrc.GetLeft(), inrc.GetBottom());
	gr->DrawLine(swrc.GetLeft(), swrc.GetBottom(), swrc.GetRight(), swrc.GetBottom());
	gr->DrawLine(swrc.GetRight(), swrc.GetTop(), swrc.GetRight(), swrc.GetBottom());

	gr->SetPen(pnWht);
	gr->DrawLine(inrc.GetLeft(), inrc.GetBottom(), inrc.GetRight(), inrc.GetBottom());
	gr->DrawLine(inrc.GetRight(), inrc.GetTop(), inrc.GetRight(), inrc.GetBottom());
	gr->DrawLine(swrc.GetLeft(), swrc.GetTop(), swrc.GetRight(), swrc.GetTop());
	gr->DrawLine(swrc.GetLeft(), swrc.GetTop(), swrc.GetLeft(), swrc.GetBottom());

	align = 1; // always center
	lblRect.x = swrc.GetLeft();
	lblRect.y = swrc.GetTop();
	lblRect.w = swrc.GetWidth();
	lblRect.h = swrc.GetHeight();
	DrawLabel(dc);
}

void *ImageWidget::LoadImage(char *file)
{
	wxImage img;
	wxString path;
	path = prjOptions.formsDir;
	path += "/";
	path += file;
	if (img.LoadFile(path))
	{
		wxBitmap *bmp = new wxBitmap(img);
		return reinterpret_cast<void*>(bmp);
	}
	return 0;
}

// todo - cache images
void ImageWidget::CreateImage()
{
	if (sipImg)
		sipImg->Release();
	sipImg = new ImageCacheItem;
	sipImg->AddRef();
	SwitchData *id = new SwitchData;
	sipImg->SetData(id);
	if (onImg)
		id->bm[1] = LoadImage(onImg);
	if (offImg)
		id->bm[0] = LoadImage(offImg);
}

void ImageWidget::Paint(DrawContext dc)
{
	SwitchWidget::Paint(dc);
	if (sipImg == NULL)
		CreateImage();
	if (sipImg)
	{
		wxBitmap *img = reinterpret_cast<wxBitmap*>(sipImg->GetImage(swOn ? 1 : 0));
		if (img)
		{
			int w = img->GetWidth();
			int h = img->GetHeight();
			int x = area.x + ((area.w - w) / 2);
			int y  = area.y + ((area.h - h) / 2);
			wxDC *gr = (wxDC*)dc;
			gr->DrawBitmap(*img, x, y, true);
		}
	}
}

void LampWidget::CreateImage() {}

void LampWidget::Paint(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;

	wdgRect baseRing;
	baseRing.x = area.x;
	baseRing.y = area.y;
	int ring;
	if (area.w > area.h)
		ring = area.h - 1;
	else
		ring = area.w - 1;
	if (!(style & 2))
	{
		baseRing.w = ring;
		baseRing.h = ring;
	}
	else
	{
		baseRing.w = area.w;
		baseRing.h = area.h;
	}

	wdgRect lamp;
	lamp = baseRing;
	lamp.Inflate(-4, -4);

	wxBrush backBrush(ColorRGB(bgClr));
	gr->SetBrush(backBrush);
	gr->SetPen(wxPen(ColorRGB(bgClr)));
	gr->DrawRectangle(area.GetLeft(), area.GetTop(), area.Width(), area.Height());
	wxPen olPen(wxColour(10,10,10));
	gr->SetPen(olPen);
	if (style & 1)
	{
		gr->GradientFillLinear(wxRect(baseRing.x, baseRing.y, baseRing.w, baseRing.h), wxColour(255,255,255), wxColour(96,96,96));
	}
	else
	{
		gr->SetBrush(wxBrush(wxColour(128,128,128)));
		gr->DrawEllipse(baseRing.GetLeft(), baseRing.GetTop(), baseRing.Width(), baseRing.Height());
	}
	if (style & 1)
	{
		if (swOn)
			gr->GradientFillLinear(wxRect(baseRing.x, baseRing.y, baseRing.w, baseRing.h), ColorRGB(hiColor), ColorRGB(loColor));
		else
			gr->GradientFillLinear(wxRect(baseRing.x, baseRing.y, baseRing.w, baseRing.h), wxColour(128,128,128), wxColour(20,20,20));
	}
	else
	{
		if (swOn)
			gr->SetBrush(wxBrush(ColorRGB(loColor)));
		else
			gr->SetBrush(wxBrush(wxColour(40,40,40)));
		gr->DrawEllipse(lamp.GetLeft(), lamp.GetTop(), lamp.Width(), lamp.Height());
	}
	DrawLabel(dc);
}

void BoxWidget::Paint(DrawContext dc)
{
	if (!show)
		return;

	wxDC *gr = (wxDC*)dc;
	wxBrush bk(ColorRGB(bgClr));
	if (!filled)
		bk.SetStyle(wxTRANSPARENT);
	gr->SetBrush(bk);
	wxPen fgPen(ColorRGB(fgClr), (int)thick);
	gr->SetPen(fgPen);
	gr->DrawRectangle(area.x, area.y, area.w, area.h);
	if (style != 0)
	{
		wxPen hiPen(ColorRGB(hiClr), (int)thick);
		wxPen loPen(ColorRGB(loClr), (int)thick);
		wxPen *p1, *p2;
		if (style == 1) // inset
		{
			p1 = &loPen;
			p2 = &hiPen;
		}
		else // outset
		{
			p1 = &hiPen;
			p2 = &loPen;
		}
		if (area.w > 1)
		{
			gr->SetPen(*p1);
			gr->DrawLine(area.GetLeft(), area.GetTop(), area.GetRight(), area.GetTop());
			gr->SetPen(*p2);
			gr->DrawLine(area.GetLeft(), area.GetBottom(), area.GetRight(), area.GetBottom());
		}
		if (area.h > 1)
		{
			gr->SetPen(*p1);
			gr->DrawLine(area.GetLeft(), area.GetTop(), area.GetLeft(), area.GetBottom());
			gr->SetPen(*p2);
			gr->DrawLine(area.GetRight(), area.GetTop(), area.GetRight(), area.GetBottom());
		}
	}
}

void TextWidget::Paint(DrawContext dc)
{
	if (text && show)
	{
		wxDC *gr = (wxDC*)dc;
		if (filled)
		{
			gr->SetPen(wxPen(*wxBLACK, 0, wxTRANSPARENT));
			gr->SetBrush(wxBrush(ColorRGB(bgClr)));
			gr->DrawRectangle(area.x, area.y, area.w, area.h);
		}

		int th = (textHeight * 72) / 96;
		wxFont font(th, wxFONTFAMILY_SWISS,
			textItalic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			textBold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);

		gr->SetFont(font);
		gr->SetTextForeground(ColorRGB(fgClr));
		wxString grText((const char*)text);
		wxCoord width, height, descent, externalLeading;
		gr->GetTextExtent(grText, &width, &height, &descent, &externalLeading);

		wxCoord tx;
		wxCoord ty;
		if (align == 0)
			tx = (wxCoord)area.x;
		else if (align == 1)
			tx = (wxCoord)area.x + ((area.w - width) / 2.0);
		else
			tx = (wxCoord)(area.x + area.w) - width;
		ty = (wxCoord)area.y + ((area.h - height) / 2.0);
		if (inset)
		{
			gr->SetFont(font);
			gr->SetTextForeground(*wxBLACK);
			gr->DrawText(grText, tx-inset,ty-inset);
			gr->SetFont(font);
			gr->SetTextForeground(*wxWHITE);
			gr->DrawText(grText, tx+inset,ty+inset);
		}
		else if (shadow)
		{
			gr->SetFont(font);
			gr->SetTextForeground(*wxBLACK);
			gr->DrawText(grText, tx+shadow,ty+shadow);
		}
		gr->SetFont(font);
		gr->SetTextForeground(wxColour(ColorRGB(fgClr)));
		gr->DrawText(grText, tx, ty);
	}
}

void GraphWidget::Paint(DrawContext dc)
{
	if (!show)
		return;

	wxDC *gr = (wxDC*)dc;

	wxBrush bk(ColorRGB(bgClr));
	wxPen pn(ColorRGB(fgClr), 1);
	gr->SetBrush(bk);
	gr->SetPen(pn);
	gr->DrawRectangle(area.x, area.y, area.w, area.h);

	if (numVals < 1 || vals == 0 || range == 0)
		return;

	wxDouble scale = (wxDouble) (area.h - 2) / range;
	wxDouble incr = (wxDouble) (area.w - 2) / (wxDouble) numVals;
	if (bar)
	{
		gr->SetBrush(wxBrush(ColorRGB(grClr)));
		gr->SetPen(wxPen(ColorRGB(grClr)));

		wxDouble xx = area.x + 1;
		wxDouble ww = incr * barWidth;
		for (int n = 0; n < numVals; n++)
		{
			wxDouble h = vals[n] * scale;
			gr->DrawRectangle(xx, area.GetBottom() - 1 - h, ww, h);
			xx += incr;
		}
	}
	else
	{
		wxPen gpn(ColorRGB(grClr), (int)thick);
		wxDouble x1 = area.x + 1;
		wxDouble y1 = area.GetBottom();
		wxDouble x2, y2;
		for (int n = 0; n < numVals; n++)
		{
			wxDouble y = vals[n] * scale;
			x2 = x1 + incr;
			y2 = area.GetBottom() - 1 - y;
			gr->DrawLine(x1, y1, x2, y2);
			x1 = x2;
			y1 = y2;
		}
	}
}

void EnvelopeWidget::Paint(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;

	wxBrush bk(ColorRGB(bgClr));
	wxPen fr(ColorRGB(frClr), 1);
	gr->SetBrush(bk);
	if (!border)
		fr.SetStyle(wxTRANSPARENT);
	gr->DrawRectangle(area.x, area.y, area.w, area.h);
	if (numSegs < 1 || vals == 0)
		return;
	int num = numSegs;

	wxPen pn(ColorRGB(fgClr), 1);
	gr->SetPen(pn);


	wxDouble lvlscale = (wxDouble) (area.Height() - 2) / ampRange;
	wxDouble rtscale = (wxDouble) (area.Width() - 2) / tmRange;

	wxDouble rtval = 0;
	wxDouble lvlval = 0;
	wxDouble xo = (wxDouble) area.GetLeft() + 1;
	wxDouble yo = (wxDouble) area.GetBottom() - 1;
	wxDouble x1 = xo;
	wxDouble y1 = yo - (start * lvlscale);
	wxDouble x2 = x1;
	wxDouble y2 = y1;
//	if (y1 != yo)
//		gr->DrawLine(xo, yo, xo, y1);
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
		x2 = xo + (rtval * rtscale);
		y2 = yo - (lvlval * lvlscale);
		gr->DrawLine(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
	if (susOn)
	{
		x2 = (wxDouble) area.GetRight() - (vals[ndx].rate * rtscale);
		if (x2 > x1)
		{
			gr->DrawLine(x1, y1, x2, y2);
			x1 = x2;
			x2 = (wxDouble) area.GetRight();
		}
		lvlval = vals[ndx].level;
		if (lvlval > ampRange)
			lvlval = ampRange;
		y2 = yo - (lvlval * lvlscale);
		gr->DrawLine(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
//	else if (y1 != yo)
//		gr->DrawLine(x1, y1, x1, yo);
}

void WaveWidget::Paint(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;

	wxBrush bk(ColorRGB(bgClr));
	wxPen fr(ColorRGB(frClr), 1);
	gr->SetBrush(bk);
	if (!border)
		fr.SetStyle(wxTRANSPARENT);
	gr->DrawRectangle(area.x, area.y, area.w, area.h);
	if (wvType == 1)
		PlotSum(dc);
	else if (wvType == 2)
		PlotSeg(dc);
}

void WaveWidget::PlotSum(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;
	int cx = area.w - 4;
	int cy = area.h - 4;

	if (cx > 0 && cy > 0)
	{
		int parts = 0;
		int x, n;
		wxDouble mid = (wxDouble)cy / 2.0;
		wxDouble maxmul = 1;
		wxDouble pp[WFI_MAXPART];
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

		wxDouble phsIncr = twoPI / (wxDouble) cx;
		wxDouble *ampvals = new wxDouble[cx];
		wxDouble maxAmp = 0.0;
		wxDouble sigma;
		wxDouble sigK = PI / maxmul;
		wxDouble sigN;
		for (x = 0; x < cx; x++)
		{
			ampvals[x] = 0.0;
			for (n = 0; n < WFI_MAXPART; n++)
			{
				if (!on[n])
					continue;
				if (gibbs && n > 0)
				{
					sigN = (wxDouble)n * sigK;
					sigma = amps[n] * (sin(sigN) / sigN);
				}
				else
					sigma = amps[n];
				ampvals[x] += (wxDouble) sin(pp[n]) * sigma;
				pp[n] += phsIncr * (wxDouble) (n+1);
				if (pp[n] >= twoPI)
					pp[n] -= twoPI;
			}
			if (fabs(ampvals[x]) > maxAmp)
				maxAmp = fabs(ampvals[x]);
		}
		wxDouble xo = (wxDouble) area.x + 2;
		wxDouble yo = (wxDouble) area.y + 2 + mid;
		wxDouble x1 = xo;
		wxDouble y1 = yo - ((ampvals[0] / maxAmp) * mid);
		wxDouble x2, y2;
		wxPen fr(ColorRGB(frClr), 1);
		gr->SetPen(fr);
		gr->DrawLine(xo, yo, (wxDouble) area.GetRight() - 2, yo);
		wxPen pn(ColorRGB(fgClr), 1);
		gr->SetPen(pn);
		for (x = 1; x < cx; x++)
		{
			x2 = x1 + 1;
			y2 = yo - ((ampvals[x] / maxAmp) * mid);
			gr->DrawLine(x1, y1, x2, y2);
			x1 = x2;
			y1 = y2;
		}
		x2 = x1 + 1;
		y2 = yo - ((ampvals[0] / maxAmp) * mid);
		gr->DrawLine(x1, y1, x2, y2);
		delete ampvals;
	}
}

void WaveWidget::PlotSeg(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;

	int cx = area.w - 4;
	int cy = area.h - 4;

	if (cx > 0 && cy > 0)
	{
		int n;
		wxDouble cxF = (wxDouble) cx;
		wxDouble cyF = (wxDouble) cy;
		wxDouble top = (wxDouble) area.y + 2;
		wxDouble bot = top + cyF;
		wxDouble mid = cyF / 2.0;
		wxDouble rgt = (wxDouble) (area.x + 2) + cxF;
		wxDouble yo = (wxDouble) area.y + 2 + mid;
		wxDouble x1 = (wxDouble) area.x + 2;
		wxDouble y1 = yo;
		wxDouble x2 = rgt;
		wxDouble y2 = yo;
		wxPen pn(ColorRGB(fgClr), 1);
		wxPen fr(ColorRGB(frClr), 1);
		gr->SetPen(fr);
		gr->DrawLine(x1, y1, x2, y2); // center line
		gr->SetPen(pn);
		for (n = 0; n < WFI_MAXPART; n++)
		{
			if (!on[n])
				continue;
			x2 = x1 + (phs[n] * cxF);
			if (x2 > rgt)
				x2 = rgt;
			y2 = yo - (amps[n] * mid);
			if (y2 < top)
				y2 = top;
			else if (y2 > bot)
				y2 = bot;
			gr->DrawLine(x1, y1, x2, y2);
			x1 = x2;
			y1 = y2;
		}
	}
}

void SliderWidget::CreateImage()
{
}

void SliderWidget::Paint(DrawContext dc)
{
	wxDC *gr = (wxDC*)dc;

	wxPen pnBlk(wxColour(40,40,40));
	wxPen pnWht(wxColour(220,220,220));
	wxPen pnTrk(wxColour(10,20,20), 3);
	pnBlk.SetCap(wxCAP_PROJECTING);
	pnWht.SetCap(wxCAP_PROJECTING);

	gr->SetPen(pnBlk);
	gr->GradientFillLinear(wxRect(area.GetLeft(), area.GetTop(), area.Width(), area.Height()),
		wxColour(120,120,120), wxColour(80,80,80));
	gr->SetPen(pnWht);
	gr->DrawLine(area.GetLeft(), area.GetTop(), area.GetRight(), area.GetTop());
	gr->DrawLine(area.GetLeft(), area.GetTop(), area.GetLeft(), area.GetBottom());

	gr->SetPen(pnTrk);
	if (vertical)
		gr->DrawLine(slideTrack.x, slideTrack.y, slideTrack.x, slideTrack.GetBottom());
	else
		gr->DrawLine(slideTrack.x, slideTrack.y, slideTrack.GetRight(), slideTrack.y);

	wxBrush knobBrush(wxColour(40,40,40), vertical ? wxHORIZONTAL_HATCH : wxVERTICAL_HATCH);
	gr->SetBrush(knobBrush);
	gr->SetPen(pnBlk);
	gr->DrawRectangle(knobPos.GetLeft(), knobPos.GetTop(), knobPos.Width(), knobPos.Height());

	int mid;
	if (vertical)
	{
		mid = knobPos.GetTop() + (knobPos.Height()/2);
		gr->DrawLine(knobPos.GetLeft(), mid, knobPos.GetRight(), mid);
	}
	else
	{
		mid = knobPos.GetLeft() + (knobPos.Width()/2);
		gr->DrawLine(mid, knobPos.GetTop(), mid, knobPos.GetBottom());
	}
}
#endif
