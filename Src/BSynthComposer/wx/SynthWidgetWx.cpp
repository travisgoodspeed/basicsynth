//////////////////////////////////////////////////////////////////////
// Platform-specifc graphics code for wxWidgets
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#if wxUSE_GRAPHICS_CONTEXT

// Wx colors have RED in the LSB...
inline long SwapRGB(long c)
{
	return ((c>>16)&0x0000ff)|(c&0x00ff00)|((c<<16)&0xff0000);
}

inline wxColour ColorRGB(long c)
{
	return wxColour((c>>16)&0xff,(c>>8)&0xff,c&0xff,(c>>24)&0xff);
}

void WidgetGroup::Paint(DrawContext dc)
{
	wxGraphicsContext *gr = (wxGraphicsContext *)dc;
	gr->SetBrush(wxBrush(ColorRGB(bgClr)));
	if (border && show)
	{
		gr->SetPen(wxPen(ColorRGB(frClr), (int)border));
		gr->DrawRectangle(area.x, area.y, area.w, area.h);
		if (style != 0)
		{
			gr->SetPen(wxPen(ColorRGB(fgClr), (int)border));
			if (style == 1) // inset
			{
				if (area.w > 1)
					gr->StrokeLine(area.GetLeft(), area.GetBottom(), area.GetRight(), area.GetBottom());
				if (area.h > 1)
					gr->StrokeLine(area.GetRight(), area.GetTop(), area.GetRight(), area.GetBottom());
			}
			else // outset
			{
				if (area.w > 1)
					gr->StrokeLine(area.GetLeft(), area.GetTop(), area.GetRight(), area.GetTop());
				if (area.h > 1)
					gr->StrokeLine(area.GetLeft(), area.GetTop(), area.GetLeft(), area.GetBottom());
			}
		}
	}
	else
	{
		gr->SetPen(wxNullPen);
		gr->DrawRectangle(area.x, area.y, area.w, area.h);
	}

	if (show)
		EnumList(EnumPaint, dc);
}

// Create/Destroy image functions are used to
// implement a bitmap cache for knobs and switches.
// A widget form typically has multiple knobs and
// switches with the same size and attributes.
// By using a bitmap we only draw the image once
// and can then bitblt the image multiple times.
// Because knobs and switches use a lot of gradient
// fills, we see a noticiable improvement in drawing speed.
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

void KnobData::DestroyImage() 
{
	if (bm)
	{
		delete (wxBitmap*)bm;
		bm = 0;
	}
}

void KnobWidget::CreateImage() 
{
	if (!GetKnobImage(plainKnob))
	{
		wdgRect back(0, 0, area.w, area.h);
		wdgRect faceDial(dial.x - area.x, dial.y - area.y, dial.w, dial.h);

		wxBitmap *bm = (wxBitmap*)knbImg->GetImage(0);
		wxMemoryDC *mdc = new wxMemoryDC(*bm);
		wxGraphicsContext *gr = wxGraphicsContext::Create(*mdc);
		wxBrush bck(ColorRGB(bgClr));
		gr->SetBrush(bck);
		gr->SetPen(wxNullPen);
		gr->DrawRectangle(0, 0, area.Width(), area.Height());
		if (tickLen)
			DrawTicks(gr, back, faceDial);

		wxPen indPen(ColorRGB(fgClr), 2);
		gr->SetPen(indPen);
#if wxMAJOR_VERSION >= 2 && wxMINOR_VERSION >= 9
		wxGraphicsGradientStops faceStops;
		faceStops.Add(ColorRGB(faceClr), 0.0f);
		faceStops.Add(ColorRGB(highClr), 0.5f);
		faceStops.Add(ColorRGB(faceClr), 1.0f);
		wxGraphicsBrush br = gr->CreateLinearGradientBrush(
			faceDial.GetLeft(), faceDial.GetTop(), 
			faceDial.GetRight(), faceDial.GetBottom(),
			faceStops);
#else
		wxGraphicsBrush br = gr->CreateLinearGradientBrush(
			faceDial.GetLeft(), faceDial.GetTop(), 
			faceDial.GetRight(), faceDial.GetBottom(),
			ColorRGB(highClr), ColorRGB(faceClr));
#endif
		gr->SetBrush(br);
		gr->DrawEllipse(faceDial.GetLeft(), faceDial.GetTop(), faceDial.Width(), faceDial.Height());
		delete gr;
		delete mdc;
	}
}

void KnobWidget::Paint(DrawContext dc)
{
	wxGraphicsContext *gr = (wxGraphicsContext *)dc;
	if (!knbImg)
		CreateImage();

	if (enable)
	{
		wxPen indPen(ColorRGB(fgClr), 2);
		gr->SetPen(indPen);
		if (knbImg)
		{
			wxBitmap *bm = (wxBitmap*)knbImg->GetImage(0);
			gr->DrawBitmap(*bm, (wxDouble)area.x, (wxDouble)area.y, (wxDouble)area.Width(), (wxDouble)area.Height());
		}
		else
			gr->DrawEllipse((wxDouble)dial.GetLeft(), (wxDouble)dial.GetTop(), (wxDouble)dial.Width(), (wxDouble)dial.Height());
		gr->StrokeLine((wxDouble)centerPt.x, (wxDouble)centerPt.y, (wxDouble)valuePt.x, (wxDouble)valuePt.y);
	}
	else
	{
		wxPen half(wxColour(0,0,0,0x80));
		gr->SetPen(half);
		gr->SetBrush(wxNullBrush);
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
	wxGraphicsContext *gr = (wxGraphicsContext*)dc;

	if (tickMaj > 0 && tickLen > 0)
	{
		wxPen pn(ColorRGB(fgClr));
		gr->SetPen(pn);
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
			gr->StrokeLine(x1, y1, xc, yc);
			if (tickMin > 0)
			{
				wxDouble ang2 = ang;
				for (int m = 0; m < tickMin; m++)
				{
					x1 = xc + (cos(ang2) * rx2);
					y1 = yc - (sin(ang2) * ry2);
					gr->StrokeLine(x1, y1, xc, yc);
					ang2 -= inc2;
				}
			}
			ang -= inc1;
		}
		//ang = pi125 - pi150;
		x1 = xc + (cos(ang) * rx1);
		y1 = yc - (sin(ang) * ry1);
		gr->StrokeLine(x1, y1, xc, yc);
	}
}

void KnobBlack::CreateImage() 
{
	if (!GetKnobImage(blackKnob))
	{
		wdgRect back(0, 0, area.w, area.h);
		wdgRect baseDial(shaft.x - area.x, shaft.y - area.y, shaft.w, shaft.h);
		wdgRect faceDial(dial.x - area.x, dial.y - area.y, dial.w, dial.h);

		wxBitmap *bm = (wxBitmap*)knbImg->GetImage(0);
		wxMemoryDC *mdc = new wxMemoryDC(*bm);
		wxGraphicsContext *gr = wxGraphicsContext::Create(*mdc);

		wxPen olPen(wxColour(0,0,0), 2);
		wxBrush backBrush(ColorRGB(bgClr));
		wxBrush baseBrush(wxColour(0,0,0),wxVERTICAL_HATCH);
#if wxMAJOR_VERSION >= 2 && wxMINOR_VERSION >= 9
		wxGraphicsGradientStops faceStops;
		faceStops.Add(ColorRGB(faceClr), 0.0f);
		faceStops.Add(ColorRGB(highClr), 0.5f);
		faceStops.Add(ColorRGB(faceClr), 1.0f);
		wxGraphicsBrush topBrush = gr->CreateLinearGradientBrush(
			faceDial.GetLeft(), faceDial.GetTop(), 
			faceDial.GetRight(), faceDial.GetBottom(),
			faceStops);
#else
		wxGraphicsBrush topBrush = gr->CreateLinearGradientBrush(
			faceDial.GetLeft(), faceDial.GetTop(),
			faceDial.GetRight(), faceDial.GetBottom(),
			ColorRGB(highClr), ColorRGB(faceClr));
#endif
		gr->SetPen(wxNullPen);
		gr->SetBrush(backBrush);
		gr->DrawRectangle(back.GetLeft(), back.GetTop(), back.Width(), back.Height());
		if (tickLen)
			DrawTicks(gr, back, baseDial);
		gr->SetPen(wxNullPen);
		gr->SetBrush(baseBrush);
		gr->DrawEllipse(baseDial.GetLeft(), baseDial.GetTop(), baseDial.Width(), baseDial.Height());
		gr->SetPen(olPen);
		gr->SetBrush(topBrush);
		gr->DrawEllipse(faceDial.GetLeft(), faceDial.GetTop(), faceDial.Width(), faceDial.Height());
		delete gr;
		delete mdc;
	}
}

void KnobBlack::Paint(DrawContext dc)
{
	KnobWidget::Paint(dc);
}

void KnobBased::CreateImage() 
{
	if (!GetKnobImage(baseKnob))
	{
		wdgRect back(0, 0, area.w, area.h);
		wdgRect baseDial(base.x - area.x, base.y - area.y, base.w, base.h);
		wdgRect midDial(shaft.x - area.x, shaft.y - area.y, shaft.w, shaft.h);
		wdgRect topDial(dial.x - area.x, dial.y - area.y, dial.w, dial.h);

		wxBitmap *bm = (wxBitmap*)knbImg->GetImage(0);
		wxMemoryDC *mdc = new wxMemoryDC(*bm);
		wxGraphicsContext *gr = wxGraphicsContext::Create(*mdc);

		wxBrush backBrush(ColorRGB(bgClr));
		wxPen backPen(ColorRGB(bgClr));
		gr->SetBrush(backBrush);
		gr->SetPen(wxNullPen);
		gr->DrawRectangle(0, 0, area.w, area.h);
		if (tickLen)
			DrawTicks(gr, back, baseDial);

#if wxMAJOR_VERSION >= 2 && wxMINOR_VERSION >= 9
		wxGraphicsGradientStops baseStops;
		baseStops.Add(wxColour(240,240,240), 0.0f);
		baseStops.Add(wxColour(100,100,100), 1.0f);

		wxGraphicsBrush baseBrush = gr->CreateLinearGradientBrush(
			baseDial.GetLeft(), baseDial.GetTop(), 
			baseDial.GetRight(), baseDial.GetBottom(),
			baseStops);

		wxGraphicsGradientStops midStops;
		midStops.Add(wxColour(64,64,64), 0.0f);
		midStops.Add(wxColour(12,12,12), 0.65f);
		midStops.Add(wxColour(0,0,0), 1.0f);

		wxGraphicsBrush midBrush = gr->CreateLinearGradientBrush(
			midDial.GetLeft(), midDial.GetTop(), 
			midDial.GetRight(), midDial.GetBottom(),
			midStops);

		wxGraphicsGradientStops faceStops;
		faceStops.Add(ColorRGB(faceClr), 0.0f);
		faceStops.Add(ColorRGB(highClr), 0.5f);
		faceStops.Add(ColorRGB(faceClr), 1.0f);
		wxGraphicsBrush topBrush = gr->CreateLinearGradientBrush(
			topDial.GetLeft(), topDial.GetTop(), 
			topDial.GetRight(), topDial.GetBottom(),
			faceStops);
#else
		wxGraphicsBrush baseBrush = gr->CreateLinearGradientBrush(
			baseDial.GetLeft(), baseDial.GetTop(), 
			baseDial.GetRight(), baseDial.GetBottom(),
			wxColour(250,250,255), wxColour(30,30,30));
		wxGraphicsBrush midBrush = gr->CreateLinearGradientBrush(
			midDial.GetLeft(), midDial.GetTop(), 
			midDial.GetRight(), midDial.GetBottom(),
			wxColour(64,64,64), wxColour(0,0,0));
		wxGraphicsBrush topBrush = gr->CreateLinearGradientBrush(
			topDial.GetLeft(), topDial.GetTop(), 
			topDial.GetRight(), topDial.GetBottom(),
			ColorRGB(highClr), ColorRGB(faceClr));
#endif

		wxPen olPen(wxColour(10,10,10),0);
		gr->SetPen(olPen);
		gr->SetBrush(baseBrush);
		gr->DrawEllipse(baseDial.GetLeft(), baseDial.GetTop(), baseDial.Width(), baseDial.Height());
		gr->SetBrush(midBrush);
		gr->DrawEllipse(midDial.GetLeft(), midDial.GetTop(), midDial.Width(), midDial.Height());
		gr->SetBrush(topBrush);
		gr->DrawEllipse(topDial.GetLeft(), topDial.GetTop(), topDial.Width(), topDial.Height());

		delete gr;
		delete mdc;
	}
}

void KnobBased::Paint(DrawContext dc)
{
	KnobWidget::Paint(dc);
}

void SwitchWidget::CreateImage() 
{
	if (GetSwitchImage(pushSwitch))
		return;
	if (sip == 0)
		return;

	wdgRect swrc(1, 1, area.w-2, area.h-2);
	wxPen pnWht(wxColour(220,220,180));
	wxPen pnBlk(wxColour(24,24,24));
	wxBrush bck(ColorRGB(bgClr));

	wxBitmap *bmOff = (wxBitmap*)sip->GetImage(0);
	wxMemoryDC *mdc = new wxMemoryDC(*bmOff);
	wxGraphicsContext *gr = wxGraphicsContext::Create(*mdc);

	gr->SetBrush(gr->CreateLinearGradientBrush(0, 0, area.w, area.h, 
		ColorRGB(swuphi), ColorRGB(swuplo)));
	gr->SetPen(pnBlk);
	gr->DrawRectangle(0, 0, area.w, area.h);
	gr->StrokeLine(swrc.x, swrc.y + swrc.h, swrc.x + swrc.w, swrc.y + swrc.h);
	gr->StrokeLine(swrc.x + swrc.w, swrc.y, swrc.x + swrc.w, swrc.y + swrc.h);
	gr->SetPen(pnWht);
	gr->StrokeLine(swrc.x, swrc.y, swrc.x + swrc.w, swrc.y);
	gr->StrokeLine(swrc.x, swrc.y, swrc.x, swrc.y + swrc.h);
	delete gr;
	delete mdc;

	wxBitmap *bmOn = (wxBitmap*)sip->GetImage(1);
	mdc = new wxMemoryDC(*bmOn);
	gr = wxGraphicsContext::Create(*mdc);
	gr->SetBrush(gr->CreateLinearGradientBrush(0, 0, area.w, area.h, 
		ColorRGB(swdnlo), ColorRGB(swdnhi)));
	gr->SetPen(pnBlk);
	gr->DrawRectangle(0, 0, area.w, area.h);
	gr->StrokeLine(swrc.x, swrc.y, swrc.x + swrc.w, swrc.y);
	gr->StrokeLine(swrc.x, swrc.y, swrc.x, swrc.y + swrc.h);
	gr->SetPen(pnWht);
	gr->StrokeLine(swrc.x, swrc.y + swrc.h, swrc.x + swrc.w, swrc.y + swrc.h);
	gr->StrokeLine(swrc.x + swrc.w, swrc.y, swrc.x + swrc.w, swrc.y + swrc.h);

	delete gr;
	delete mdc;
}

void SwitchWidget::Paint(DrawContext dc)
{
	wxGraphicsContext *gr = (wxGraphicsContext *)dc;
	if (!sip)
		CreateImage();

	if (!show || !enable || !sip)
	{
		wxBrush bck(ColorRGB(bgClr));
		wxPen pnBlk(wxColour(24,24,24));
		if (!show)
			gr->SetPen(wxNullPen);
		else
			gr->SetPen(pnBlk);
		gr->SetBrush(bck);
		gr->DrawRectangle((wxDouble)area.x, (wxDouble)area.y, (wxDouble)area.w, (wxDouble)area.h);
	}
	else
	{
		wxBitmap *bm = reinterpret_cast<wxBitmap*>(sip->GetImage(swOn ? 1 : 0));
		if (bm)
			gr->DrawBitmap(*bm, (wxDouble)area.x, (wxDouble)area.y, (wxDouble)area.w, (wxDouble)area.h);
	}
	DrawLabel(dc);
}


void SwitchWidget::DrawLabel(DrawContext dc)
{
	if (lbl && show)
	{
		wxGraphicsContext *gr = (wxGraphicsContext *)dc;
		int th = (72 * lblHeight) / 96;
		wxFont font(th, wxFONTFAMILY_DEFAULT,
			italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL, false,
			wxString(form->LabelFont()));

		gr->SetFont(font, ColorRGB(fgClr));
		wxString grText((const char*)lbl);
		wxDouble width, height, descent, externalLeading;
		gr->GetTextExtent(grText, &width, &height, &descent, &externalLeading);

		wxDouble tx;
		wxDouble ty;
		if (align == 0)
			tx = (wxDouble)lblRect.x;
		else if (align == 1)
			tx = (wxDouble)lblRect.x + ((lblRect.w - width) / 2.0);
		else
			tx = (wxDouble)(lblRect.x + lblRect.w) - width;
		ty = (wxDouble)lblRect.y + ((lblRect.h - height) / 2.0);
		if (shadow)
		{
			gr->SetFont(font, wxColour(0,0,0));
			gr->DrawText(grText, tx+shadow, ty+shadow);
		}
		if (enable)
			gr->SetFont(font, ColorRGB(fgClr));
		else
			gr->SetFont(font, wxColour(0,0,0,0x80));
		gr->DrawText(grText, tx, ty);
	}
}

void SlideSwitchWidget::CreateImage() 
{
	if (GetSwitchImage(slideSwitch))
		return;
	if (sip == 0)
		return;

	wdgRect bgrc(0, 0, area.w, area.h);
	wdgRect inrc;
	inrc = bgrc;
	inrc.Shrink(2, 2);
	wdgRect swrc;
	swrc = inrc;
	swrc.Shrink(2, 2);
	if (style == 0) // horizontal
		swrc.w /= 2;
	else // vertical
		swrc.h /= 2;

	wxPen pnBlk(wxColor(24,24,24), 2);
	wxPen pnWht(wxColor(180,176,170), 2);
	pnBlk.SetCap(wxCAP_PROJECTING);
	pnWht.SetCap(wxCAP_PROJECTING);

	for (int pos = 0; pos < 2; pos++)
	{
		wxBitmap *bm = (wxBitmap*)sip->GetImage(pos);
		wxMemoryDC *mdc = new wxMemoryDC(*bm);
		wxGraphicsContext *gr = wxGraphicsContext::Create(*mdc);
		wxGraphicsBrush brbase = gr->CreateLinearGradientBrush(0, 0, area.w, area.h, wxColour(50,50,50), wxColour(128,128,128));
		wxGraphicsBrush brswtc = gr->CreateLinearGradientBrush(
						    swrc.GetLeft(), swrc.GetTop(), swrc.GetRight(), swrc.GetBottom(),
							wxColour(148,148,148), wxColour(64,64,64));
		gr->SetPen(pnBlk);

		// base
		gr->SetBrush(brbase);
		gr->DrawRectangle(bgrc.GetLeft(), bgrc.GetTop(), bgrc.Width(), bgrc.Height());

		// switch
		gr->SetBrush(brswtc);
		gr->DrawRectangle(swrc.GetLeft(), swrc.GetTop(), swrc.Width(), swrc.Height());

		// outline
		gr->StrokeLine(inrc.GetLeft(), inrc.GetTop(), inrc.GetRight(), inrc.GetTop());
		gr->StrokeLine(inrc.GetLeft(), inrc.GetTop(), inrc.GetLeft(), inrc.GetBottom());
		gr->StrokeLine(swrc.GetLeft(), swrc.GetBottom(), swrc.GetRight(), swrc.GetBottom());
		gr->StrokeLine(swrc.GetRight(), swrc.GetTop(), swrc.GetRight(), swrc.GetBottom());

		// highlight
		gr->SetPen(pnWht);
		gr->StrokeLine(inrc.GetLeft(), inrc.GetBottom(), inrc.GetRight(), inrc.GetBottom());
		gr->StrokeLine(inrc.GetRight(), inrc.GetTop(), inrc.GetRight(), inrc.GetBottom());
		gr->StrokeLine(swrc.GetLeft(), swrc.GetTop(), swrc.GetRight(), swrc.GetTop());
		gr->StrokeLine(swrc.GetLeft(), swrc.GetTop(), swrc.GetLeft(), swrc.GetBottom());

		delete gr;
		delete mdc;
		if (style == 0)
			swrc.x += swrc.w;
		else
			swrc.y += swrc.h;
	}
}

void SlideSwitchWidget::Paint(DrawContext dc)
{
	align = 1; // always center
	lblRect = area;
	lblRect.Shrink(4, 4);
	if (style == 0)
	{
		lblRect.w /= 2;
		if (swOn)
			lblRect.x += lblRect.w;
	}
	else
	{
		lblRect.h /= 2;
		if (swOn)
			lblRect.y += lblRect.h;
	}
	SwitchWidget::Paint(dc);
}

void *ImageWidget::LoadImage(char *file)
{
	wxString wxpath;
	if (SynthFileExists(file))
		wxpath = file;
	else
	{
		wxpath = form->FormsDir();
		wxpath += "/";
		wxpath += file;
	}
	wxImage img;
	if (img.LoadFile(wxpath))
	{
		wxBitmap *bmp = new wxBitmap(img);
		return reinterpret_cast<void*>(bmp);
	}
	return 0;
}

// todo - cache images
void ImageWidget::CreateImage()
{
	SwitchWidget::CreateImage();
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
	if (sipImg)
	{
		wxBitmap *img = reinterpret_cast<wxBitmap*>(sipImg->GetImage(swOn ? 1 : 0));
		if (img)
		{
			int w = img->GetWidth();
			int h = img->GetHeight();
			int x = area.x + ((area.w - w) / 2);
			int y  = area.y + ((area.h - h) / 2);
			wxGraphicsContext *gr = (wxGraphicsContext *)dc;
			gr->DrawBitmap(*img, x, y, w, h);
		}
	}
}

void LampWidget::CreateImage() 
{
	if (GetSwitchImage(lampSwitch))
		return;

	int ring;
	if (area.w > area.h)
		ring = area.h - 1;
	else
		ring = area.w - 1;

	int rw, rh;
	if (style & 2)
	{
		rh = area.h;
		rw = area.w;
	}
	else
	{
		rw = rh = ring;
	}
	wdgRect baseRing(0, 0, rw, rh);

	wdgRect lamp;
	lamp = baseRing;
	lamp.Inflate(-4, -4);

	wxPen olPen(wxColour(10,10,10));
	wxBrush backBrush(ColorRGB(bgClr));

	int centerx = lamp.GetLeft() + lamp.Width() / 2;
	int centery = lamp.GetTop() + lamp.Height() / 2;

	wxGraphicsBrush baseBrush;
	wxGraphicsBrush lampBrush;

	for (int state = 0; state < 2; state++)
	{
		wxBitmap *bm = reinterpret_cast<wxBitmap*>(sip->GetImage(state));
		wxMemoryDC *mdc = new wxMemoryDC(*bm);
		wxGraphicsContext *gr = wxGraphicsContext::Create(*mdc);
		
		baseBrush = gr->CreateLinearGradientBrush(
			baseRing.GetLeft(), baseRing.GetTop(),
			baseRing.GetRight(), baseRing.GetBottom(),
			wxColour(255,255,255), wxColour(96,96,96));
		gr->SetBrush(backBrush);
		gr->SetPen(wxNullPen);
		gr->DrawRectangle(0, 0, area.Width(), area.Height());
		gr->SetBrush(baseBrush);
		gr->SetPen(olPen);
		if (style & 1)
			gr->DrawRectangle(baseRing.GetLeft(), baseRing.GetTop(), baseRing.Width(), baseRing.Height());
		else
			gr->DrawEllipse(baseRing.GetLeft(), baseRing.GetTop(), baseRing.Width(), baseRing.Height());

		if (state == 0)
			lampBrush = gr->CreateRadialGradientBrush(centerx, centery, centerx, centery,
				ring / 2, wxColour(64,64,64), wxColour(20,20,20));
		else
			lampBrush = gr->CreateRadialGradientBrush(centerx, centery, centerx, centery,
				ring / 2, ColorRGB(hiColor), ColorRGB(loColor));
		gr->SetBrush(lampBrush);
		if (style & 1)
			gr->DrawRectangle(lamp.GetLeft(), lamp.GetTop(), lamp.Width(), lamp.Height());
		else
			gr->DrawEllipse(lamp.GetLeft(), lamp.GetTop(), lamp.Width(), lamp.Height());

		delete gr;
		delete mdc;
	}

}

void LampWidget::Paint(DrawContext dc)
{
	SwitchWidget::Paint(dc);
}

void BoxWidget::Paint(DrawContext dc)
{
	if (!show)
		return;

	wxGraphicsContext *gr = (wxGraphicsContext *)dc;
	wxBrush bk(ColorRGB(bgClr));
	if (filled)
		gr->SetBrush(bk);
	else
		gr->SetBrush(wxNullBrush);
	wxPen fgPen(ColorRGB(fgClr), (int)thick);
	gr->SetPen(fgPen);
	gr->DrawRectangle(area.x, area.y, area.w, area.h);
	if (style != 0)
	{
		if (filled)
			gr->DrawRectangle(area.x, area.y, area.w, area.h);
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
			gr->StrokeLine(area.GetLeft(), area.GetTop(), area.GetRight(), area.GetTop());
			gr->SetPen(*p2);
			gr->StrokeLine(area.GetLeft(), area.GetBottom(), area.GetRight(), area.GetBottom());
		}
		if (area.h > 1)
		{
			gr->SetPen(*p1);
			gr->StrokeLine(area.GetLeft(), area.GetTop(), area.GetLeft(), area.GetBottom());
			gr->SetPen(*p2);
			gr->StrokeLine(area.GetRight(), area.GetTop(), area.GetRight(), area.GetBottom());
		}
	}
}

void TextWidget::Paint(DrawContext dc)
{
	if (text && show)
	{
		wxGraphicsContext *gr = (wxGraphicsContext *)dc;
		if (filled)
		{
			wxBrush bck(ColorRGB(bgClr));
			gr->SetPen(wxNullPen);
			gr->SetBrush(bck);
			gr->DrawRectangle(area.x, area.y, area.w, area.h);
		}

		int th = (textHeight * 72) / 96;
		wxFont font(th, wxFONTFAMILY_DEFAULT,
			textItalic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			textBold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL, false,
			wxString(form->LabelFont()));

		gr->SetFont(font, ColorRGB(fgClr));
		wxString grText((const char*)text);
		wxDouble width, height, descent, externalLeading;
		gr->GetTextExtent(grText, &width, &height, &descent, &externalLeading);

		wxDouble tx;
		wxDouble ty;
		if (align == 0)
			tx = (wxDouble)area.x;
		else if (align == 1)
			tx = (wxDouble)area.x + ((area.w - width) / 2.0);
		else
			tx = (wxDouble)(area.x + area.w) - width;
		ty = (wxDouble)area.y + ((area.h - height) / 2.0);
		if (inset)
		{
			gr->SetFont(font, *wxBLACK);
			gr->DrawText(grText, tx-inset,ty-inset);
			gr->SetFont(font, *wxWHITE);
			gr->DrawText(grText, tx+inset,ty+inset);
		}
		else if (shadow)
		{
			gr->SetFont(font, *wxBLACK);
			gr->DrawText(grText, tx+shadow,ty+shadow);
		}
		gr->SetFont(font, ColorRGB(fgClr));
		gr->DrawText(grText, tx, ty);
	}
}

void GraphWidget::Paint(DrawContext dc)
{
	if (!show)
		return;

	wxGraphicsContext *gr = (wxGraphicsContext *)dc;

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
		wxBrush br(ColorRGB(grClr));
		gr->SetBrush(br);
		gr->SetPen(wxNullPen);

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
			gr->StrokeLine(x1, y1, x2, y2);
			x1 = x2;
			y1 = y2;
		}
	}
}

void EnvelopeWidget::Paint(DrawContext dc)
{
	wxGraphicsContext *gr = (wxGraphicsContext *)dc;

	wxBrush bk(ColorRGB(bgClr));
	wxPen fr(ColorRGB(frClr), 1);
	gr->SetBrush(bk);
	if (border)
		gr->SetPen(fr);
	else
		gr->SetPen(wxNullPen);
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
//		gr->StrokeLine(xo, yo, xo, y1);
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
		gr->StrokeLine(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
	if (susOn)
	{
		x2 = (wxDouble) area.GetRight() - (vals[ndx].rate * rtscale);
		if (x2 > x1)
		{
			gr->StrokeLine(x1, y1, x2, y2);
			x1 = x2;
			x2 = (wxDouble) area.GetRight();
		}
		lvlval = vals[ndx].level;
		if (lvlval > ampRange)
			lvlval = ampRange;
		y2 = yo - (lvlval * lvlscale);
		gr->StrokeLine(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
//	else if (y1 != yo)
//		gr->StrokeLine(x1, y1, x1, yo);
}

void WaveWidget::Paint(DrawContext dc)
{
	wxGraphicsContext *gr = (wxGraphicsContext *)dc;

	wxBrush bk(ColorRGB(bgClr));
	wxPen fr(ColorRGB(frClr), 1);
	gr->SetBrush(bk);
	if (border)
		gr->SetPen(fr);
	else
		gr->SetPen(wxNullPen);
	gr->DrawRectangle(area.x, area.y, area.w, area.h);
	if (wvType == 1)
		PlotSum(dc);
	else if (wvType == 2)
		PlotSeg(dc);
}

void WaveWidget::PlotSum(DrawContext dc)
{
	wxGraphicsContext *gr = (wxGraphicsContext *)dc;
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
		gr->StrokeLine(xo, yo, (wxDouble) area.GetRight() - 2, yo);
		wxPen pn(ColorRGB(fgClr), 1);
		gr->SetPen(pn);
		for (x = 1; x < cx; x++)
		{
			x2 = x1 + 1;
			y2 = yo - ((ampvals[x] / maxAmp) * mid);
			gr->StrokeLine(x1, y1, x2, y2);
			x1 = x2;
			y1 = y2;
		}
		x2 = x1 + 1;
		y2 = yo - ((ampvals[0] / maxAmp) * mid);
		gr->StrokeLine(x1, y1, x2, y2);
		delete ampvals;
	}
}

void WaveWidget::PlotSeg(DrawContext dc)
{
	wxGraphicsContext *gr = (wxGraphicsContext *)dc;

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
		gr->StrokeLine(x1, y1, x2, y2); // center line
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
			gr->StrokeLine(x1, y1, x2, y2);
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
	wxGraphicsContext *gr = (wxGraphicsContext *)dc;

	wxPen pnBlk(wxColour(40,40,40));
	wxPen pnWht(wxColour(220,220,220));
	wxPen pnTrk(wxColour(10,20,20), 3);
	pnBlk.SetCap(wxCAP_PROJECTING);
	pnWht.SetCap(wxCAP_PROJECTING);

	wxGraphicsBrush br = gr->CreateLinearGradientBrush(
		area.GetLeft(), area.GetTop(),
		area.GetRight(), area.GetBottom(),
		wxColour(120,120,120), wxColour(80,80,80));
	gr->SetBrush(br);
	gr->SetPen(pnBlk);
	gr->DrawRectangle(area.GetLeft(), area.GetTop(), area.Width(), area.Height());
	gr->SetPen(pnWht);
	gr->StrokeLine(area.GetLeft(), area.GetTop(), area.GetRight(), area.GetTop());
	gr->StrokeLine(area.GetLeft(), area.GetTop(), area.GetLeft(), area.GetBottom());

	gr->SetPen(pnTrk);
	if (vertical)
		gr->StrokeLine(slideTrack.x, slideTrack.y, slideTrack.x, slideTrack.GetBottom());
	else
		gr->StrokeLine(slideTrack.x, slideTrack.y, slideTrack.GetRight(), slideTrack.y);

	wxBrush knobBrush(wxColour(40,40,40), vertical ? wxHORIZONTAL_HATCH : wxVERTICAL_HATCH);
	gr->SetBrush(knobBrush);
	gr->SetPen(pnBlk);
	gr->DrawRectangle(knobPos.GetLeft(), knobPos.GetTop(), knobPos.Width(), knobPos.Height());

	int mid;
	if (vertical)
	{
		mid = knobPos.GetTop() + (knobPos.Height()/2);
		gr->StrokeLine(knobPos.GetLeft(), mid, knobPos.GetRight(), mid);
	}
	else
	{
		mid = knobPos.GetLeft() + (knobPos.Width()/2);
		gr->StrokeLine(mid, knobPos.GetTop(), mid, knobPos.GetBottom());
	}
}
#endif
