//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
// Platform-specifc graphics code

#include "stdafx.h"
#include "resource.h"
#include "QueryValueDlg.h"

// this is used to convert text. Only one string at a time
// since this utilizes a shared buffer.
static const wchar_t *WideChar(const char *in)
{
	static wchar_t *namebuffer = 0;
	static size_t namelen = 0;
	size_t inlen = strlen(in);
	if (inlen >= namelen)
	{
		delete namebuffer;
		namebuffer = new wchar_t[inlen+1];
		namelen = inlen+1;
	}
	int len = MultiByteToWideChar(CP_UTF8, 0, in, (int)inlen, namebuffer, (int)namelen-1);
	namebuffer[len] = 0;
	return namebuffer;
}

void WidgetGroup::Paint(DrawContext dc)
{
	Graphics *gr = (Graphics *)dc;
	SolidBrush bck(bgClr);
	gr->FillRectangle(&bck, area.x, area.y, area.w, area.h);
	if (border)
	{
		Pen pn(frClr, border);
		gr->DrawRectangle(&pn, area.x, area.y, area.w, area.h);
		if (style != 0)
		{
			Pen hi(fgClr, border);
			if (style == 1) // inset
			{
				if (area.w > 1)
					gr->DrawLine(&hi, area.GetLeft(), area.GetBottom(), area.GetRight(), area.GetBottom());
				if (area.h > 1)
					gr->DrawLine(&hi, area.GetRight(), area.GetTop(), area.GetRight(), area.GetBottom());
			}
			else // outset
			{
				if (area.w > 1)
					gr->DrawLine(&hi, area.GetLeft(), area.GetTop(), area.GetRight(), area.GetTop());
				if (area.h > 1)
					gr->DrawLine(&hi, area.GetLeft(), area.GetTop(), area.GetLeft(), area.GetBottom());
			}
		}
	}
	if (show)
		EnumList(EnumPaint, dc);
}

void SwitchData::DestroyImage()
{
	delete (Bitmap*)bm[0];
	delete (Bitmap*)bm[1];
	bm[0] = 0;
	bm[1] = 0;
	//printf("Destroy switch %08x\n", (long) this);
}

void SwitchData::CreateImage()
{
	DestroyImage();
	Bitmap *tmp = new Bitmap(cx, cy);
	bm[0] = (void *)tmp;
	tmp = new Bitmap(cx, cy);
	bm[1] = (void *)tmp;
}

void KnobData::CreateImage()
{
	DestroyImage();
	Bitmap *tmp = new Bitmap(cx, cy);
	bm = (void*)tmp;
}

void KnobData::DestroyImage()
{
	if (bm)
		delete (Bitmap*)bm;
	bm = 0;
}

void KnobWidget::CreateImage()
{
	if (!GetKnobImage(plainKnob))
	{
		Rect back;
		back.X = 0;
		back.Y = 0;
		back.Width = area.w;
		back.Height = area.h;

		Rect face;
		face.X = dial.x - area.x;
		face.Y = dial.y - area.y;
		face.Width = dial.w;
		face.Height = dial.h;

		Graphics *gr = Graphics::FromImage((Bitmap*)knbImg->GetImage(0));
		gr->SetSmoothingMode(SmoothingModeHighQuality);

		Pen olPen(fgClr, 4.0);
		SolidBrush backBrush(bgClr);
		LinearGradientBrush dialBrush(face, faceClr, highClr, faceAng);
		dialBrush.SetBlend(faceAmt, facePos, 3);

		gr->FillRectangle(&backBrush, back);
		if (tickLen)
			DrawTicks(gr, area, dial);
		gr->FillEllipse(&dialBrush, face);
		gr->DrawEllipse(&olPen, face);
		delete gr;
	}
}

void KnobWidget::Paint(DrawContext dc)
{
	Graphics *gr = (Graphics*)dc;
	if (!knbImg)
		CreateImage();
	gr->SetSmoothingMode(SmoothingModeHighQuality);
	if (enable)
	{
		Pen indPen(fgClr, 2.0);
		if (knbImg)
			gr->DrawImage((Bitmap*)knbImg->GetImage(0), area.x, area.y);
		else
			gr->DrawEllipse(&indPen, dial.x, dial.y, dial.w, dial.h);
		gr->DrawLine(&indPen, centerPt.x, centerPt.y, valuePt.x, valuePt.y);
	}
	else
	{
		SolidBrush bck(bgClr);
		gr->FillRectangle(&bck, area.x, area.y, area.w, area.h);
		Pen half((ARGB)0x80000000);
		gr->DrawEllipse(&half, area.x+2, area.y+2, area.w-4, area.h-4);
	}
	if (focus)
	{
		Pen olPen(fgClr, 1.0);
		gr->DrawRectangle(&olPen, area.x+1, area.y+1, area.w-2, area.h-2);
	}
}

void KnobWidget::DrawTicks(DrawContext dc, wdgRect& bk, wdgRect& fc)
{
	Graphics *gr = (Graphics *)dc;

	if (tickMaj > 0 && tickLen > 0)
	{
		Pen pn(fgClr, 0); 
		REAL hw = (REAL) bk.w / 2.0;
		REAL hh = (REAL) bk.h/ 2.0;
		REAL xc = fc.x + ((REAL)fc.w / 2.0);
		REAL yc = fc.y + ((REAL)fc.h / 2.0);
		double rx1 = hw;
		double rx2 = rx1 - (tickLen/2);
		double ry1 = hh;
		double ry2 = ry1 - (tickLen/2);
		double inc1 = pi150 / (double) tickMaj;
		double inc2 = inc1;
		if (tickMin > 0)
			inc2 /= (double) tickMin;
		REAL x1, y1;
		double ang = pi125;
		for (int n = 0; n < tickMaj; n++)
		{
			ang = pi125 - (n * inc1);
			x1 = xc + (cos(ang) * rx1);
			y1 = yc - (sin(ang) * ry1);
			gr->DrawLine(&pn, x1, y1, xc, yc);
			if (tickMin > 0)
			{
				double ang2 = ang;
				for (int m = 0; m < tickMin; m++)
				{
					x1 = xc + (cos(ang2) * rx2);
					y1 = yc - (sin(ang2) * ry2);
					gr->DrawLine(&pn, x1, y1, xc, yc);
					ang2 -= inc2;
				}
			}
			ang -= inc1;
		}
		//ang = pi125 - pi150;
		x1 = xc + (cos(ang) * rx1);
		y1 = yc - (sin(ang) * ry1);
		gr->DrawLine(&pn, x1, y1, xc, yc);
	}
}

void KnobBlack::CreateImage()
{
	if (!GetKnobImage(blackKnob))
	{
		Rect back;
		back.X = 0;
		back.Y = 0;
		back.Width = area.w;
		back.Height = area.h;

		Rect baseDial;
		baseDial.X = shaft.x - area.x;
		baseDial.Y = shaft.y - area.y;
		baseDial.Width = shaft.w;
		baseDial.Height = shaft.h;

		Rect faceDial;
		faceDial.X = dial.x - area.x;
		faceDial.Y = dial.y - area.y;
		faceDial.Width = dial.w;
		faceDial.Height = dial.h;

		Graphics *gr = Graphics::FromImage((Bitmap*)knbImg->GetImage(0));
		gr->SetSmoothingMode(SmoothingModeHighQuality);

		HatchBrush baseBrush(HatchStyleDarkVertical, Color(40, 40, 40), Color(80,80,80));
		LinearGradientBrush topBrush(faceDial, faceClr, highClr, faceAng);
		topBrush.SetBlend(faceAmt, facePos, 3);
		Pen olPen(Color(0,0,0), 4.0);

		SolidBrush backBrush(bgClr);
		gr->FillRectangle(&backBrush, back);
		if (tickLen)
			DrawTicks(gr, area, shaft);
		gr->FillEllipse(&baseBrush, baseDial);
		gr->FillEllipse(&topBrush, faceDial);
		gr->DrawEllipse(&olPen, faceDial);
		delete gr;
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
		Rect back;
		back.X = 0;
		back.Y = 0;
		back.Width = area.w;
		back.Height = area.h;

		Rect baseDial;
		baseDial.X = base.x - area.x;
		baseDial.Y = base.y - area.y;
		baseDial.Width = base.w;
		baseDial.Height = base.h;

		Rect midDial;
		midDial.X = shaft.x - area.x;
		midDial.Y = shaft.y - area.y;
		midDial.Width = shaft.w;
		midDial.Height = shaft.h;

		Rect topDial;
		topDial.X = dial.x - area.x;
		topDial.Y = dial.y - area.y;
		topDial.Width = dial.w;
		topDial.Height = dial.h;

		Graphics *gr = Graphics::FromImage((Bitmap*)knbImg->GetImage(0));
		gr->SetSmoothingMode(SmoothingModeHighQuality);

		LinearGradientBrush baseBrush(baseDial, Color(240,240,240), Color(100,100,100), 45.0);
		LinearGradientBrush midBrush(midDial, Color(250,250,255), Color(0,0,0), 45.0);
		REAL fac[] = { 0.0f, 0.0f, 0.65f, 1.0f };
		REAL pos[] = { 0.0f, 0.14f, 0.2f, 1.0f };
		midBrush.SetBlend(fac, pos, 4);
		LinearGradientBrush topBrush(topDial, faceClr, highClr, faceAng);
		topBrush.SetBlend(faceAmt, facePos, 3);
		Pen olPen(Color(10,10,10), 0.0);
		SolidBrush backBrush(bgClr);

		gr->FillRectangle(&backBrush, back);
		if (tickLen)
			DrawTicks(gr, area, base);
		gr->FillEllipse(&baseBrush, baseDial);
		gr->DrawEllipse(&olPen, baseDial);
		gr->FillEllipse(&midBrush, midDial);
		gr->DrawEllipse(&olPen, midDial);
		gr->FillEllipse(&topBrush, topDial);
		//gr->DrawEllipse(&olPen, topDial);
		delete gr;
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

	Rect bgrc;
	Rect swrc;
	bgrc.X = 0;
	bgrc.Y = 0;
	bgrc.Width = area.w;
	bgrc.Height = area.h;
	swrc = bgrc;
	swrc.Inflate(-1, -1);

	SolidBrush bck(bgClr);
	SolidBrush fore(fgClr);
	Pen pnBlk(Color(24,24,24), 1.0f);
	Pen pnWht(Color(220,220,180), 1.0f);
//	LinearGradientBrush lgb(bgrc, Color(180,180,180), Color(90,90,85), 80.0f);
	LinearGradientBrush lgb(bgrc, Color(swuphi), Color(swuplo), 80.0f);

	Graphics *grOff = Graphics::FromImage((Bitmap*)sip->GetImage(0));
	Graphics *grOn = Graphics::FromImage((Bitmap*)sip->GetImage(1));

	grOff->FillRectangle(&lgb, bgrc);
	grOff->DrawRectangle(&pnBlk, bgrc);
	grOff->DrawLine(&pnBlk, swrc.X, swrc.Y + swrc.Height, swrc.X + swrc.Width, swrc.Y + swrc.Height);
	grOff->DrawLine(&pnBlk, swrc.X + swrc.Width, swrc.Y, swrc.X + swrc.Width, swrc.Y + swrc.Height);
	grOff->DrawLine(&pnWht, swrc.X, swrc.Y, swrc.X + swrc.Width, swrc.Y);
	grOff->DrawLine(&pnWht, swrc.X, swrc.Y, swrc.X, swrc.Y + swrc.Height);

//	lgb.SetLinearColors(Color(50,50,50),Color(128,128,120));
	lgb.SetLinearColors(Color(swdnlo), Color(swdnhi));
	grOn->FillRectangle(&lgb, bgrc);
	grOn->DrawRectangle(&pnBlk, bgrc);
	grOn->DrawLine(&pnWht, swrc.X, swrc.Y + swrc.Height, swrc.X + swrc.Width, swrc.Y + swrc.Height);
	grOn->DrawLine(&pnWht, swrc.X + swrc.Width, swrc.Y, swrc.X + swrc.Width, swrc.Y + swrc.Height);
	grOn->DrawLine(&pnBlk, swrc.X, swrc.Y, swrc.X + swrc.Width, swrc.Y);
	grOn->DrawLine(&pnBlk, swrc.X, swrc.Y, swrc.X, swrc.Y + swrc.Height);

	delete grOff;
	delete grOn;
}

void SwitchWidget::Paint(DrawContext dc)
{
	Graphics *gr = (Graphics *)dc;
	if (!sip)
		CreateImage();

	if (!show || !enable || !sip)
	{
		SolidBrush bck(bgClr);
		gr->FillRectangle(&bck, area.x, area.y, area.w, area.h);
		if (show)
		{
			//Pen pn(fgClr, 1);
			Pen pn((ARGB)0x80000000);
			gr->DrawRectangle(&pn, area.x, area.y, area.w, area.h);
		}
	}
	else if (swOn)
		gr->DrawImage((Bitmap*)sip->GetImage(1), area.x, area.y);
	else
		gr->DrawImage((Bitmap*)sip->GetImage(0), area.x, area.y);
	DrawLabel(dc);
}

void SwitchWidget::DrawLabel(DrawContext dc)
{
	if (lbl && show)
	{
		Graphics *gr = (Graphics *)dc;
		RectF rcf;
		rcf.X = lblRect.x;
		rcf.Y = lblRect.y;
		rcf.Width = lblRect.w;
		rcf.Height = lblRect.h;
		FontFamily lblff(WideChar(form->LabelFont()));
		StringFormat lblFmt;
		int lblStyle = FontStyleRegular;
		if (bold)
			lblStyle |= FontStyleBold;
		if (italic)
			lblStyle |= FontStyleItalic;
		if (align == 0)
			lblFmt.SetAlignment(StringAlignmentNear);
		else if (align == 1)
			lblFmt.SetAlignment(StringAlignmentCenter);
		else
			lblFmt.SetAlignment(StringAlignmentFar);
		lblFmt.SetLineAlignment(StringAlignmentCenter);
		Font lblFont(&lblff, lblHeight, lblStyle, UnitPixel);
		const wchar_t *lblw = WideChar(lbl);
		if (shadow)
		{
			SolidBrush blk(Color::Black);
			RectF sr;
			sr.X = rcf.X + shadow;
			sr.Y = rcf.Y + shadow;
			sr.Width = rcf.Width;
			sr.Height = rcf.Height;
			gr->DrawString(lblw, -1, &lblFont, sr, &lblFmt, &blk);
		}
		ARGB c;
		if (enable)
			c = fgClr;
		else
			c = 0x80000000;
		SolidBrush br(c);
		gr->DrawString(lblw, -1, &lblFont, rcf, &lblFmt, &br);
	}
}

void SlideSwitchWidget::CreateImage()
{
	if (GetSwitchImage(slideSwitch))
		return;
	if (!sip)
		return;

	Rect bgrc;
	Rect inrc;
	Rect swrc;
	bgrc.X = 0;
	bgrc.Y = 0;
	bgrc.Width = area.w;
	bgrc.Height = area.h;
	inrc = bgrc;
	inrc.Inflate(-2, -2);
	swrc = inrc;
	swrc.Inflate(-2, -2);
	if (style == 0)
	{	// horizontal
		swrc.Width /= 2;
	}
	else
	{
		// vertical
		swrc.Height /= 2;
	}

//	SolidBrush bck(bgClr);
	LinearGradientBrush bck(bgrc, Color(50,50,50), Color(128,128,120), 80.0f);
	Pen pnBlk(Color(24,24,24), 1.8f);
	Pen pnWht(Color(180,176,170), 1.8f);

	Graphics *gr;
	
	gr = Graphics::FromImage((Bitmap*)sip->GetImage(0));
	gr->SetSmoothingMode(SmoothingModeHighQuality);
	gr->FillRectangle(&bck, bgrc);
	//gr->DrawRectangle(&pnBlk, bgrc);
	gr->DrawLine(&pnWht, inrc.X, inrc.Y + inrc.Height, inrc.X + inrc.Width, inrc.Y + inrc.Height);
	gr->DrawLine(&pnWht, inrc.X + inrc.Width, inrc.Y, inrc.X + inrc.Width, inrc.Y + inrc.Height);
	gr->DrawLine(&pnBlk, inrc.X, inrc.Y, inrc.X + inrc.Width, inrc.Y);
	gr->DrawLine(&pnBlk, inrc.X, inrc.Y, inrc.X, inrc.Y + inrc.Height);

	LinearGradientBrush lgb1(swrc, Color(148,148,140), Color(64,64,64), 80.0f);
	gr->FillRectangle(&lgb1, swrc);
	gr->DrawLine(&pnBlk, swrc.X, swrc.Y + swrc.Height, swrc.X + swrc.Width, swrc.Y + swrc.Height);
	gr->DrawLine(&pnBlk, swrc.X + swrc.Width, swrc.Y, swrc.X + swrc.Width, swrc.Y + swrc.Height);
	gr->DrawLine(&pnWht, swrc.X, swrc.Y, swrc.X + swrc.Width, swrc.Y);
	gr->DrawLine(&pnWht, swrc.X, swrc.Y, swrc.X, swrc.Y + swrc.Height);
	delete gr;

	gr = Graphics::FromImage((Bitmap*)sip->GetImage(1));
	if (style == 1)
		swrc.Y += swrc.Height;
	else
		swrc.X += swrc.Width;

	gr->FillRectangle(&bck, bgrc);
	//gr->DrawRectangle(&pnBlk, bgrc);
	gr->DrawLine(&pnWht, inrc.X, inrc.Y + inrc.Height, inrc.X + inrc.Width, inrc.Y + inrc.Height);
	gr->DrawLine(&pnWht, inrc.X + inrc.Width, inrc.Y, inrc.X + inrc.Width, inrc.Y + inrc.Height);
	gr->DrawLine(&pnBlk, inrc.X, inrc.Y, inrc.X + inrc.Width, inrc.Y);
	gr->DrawLine(&pnBlk, inrc.X, inrc.Y, inrc.X, inrc.Y + inrc.Height);

	LinearGradientBrush lgb2(swrc, Color(148,148,140), Color(64,64,64), 80.0f);
	gr->FillRectangle(&lgb2, swrc);
	gr->DrawLine(&pnBlk, swrc.X, swrc.Y + swrc.Height, swrc.X + swrc.Width, swrc.Y + swrc.Height);
	gr->DrawLine(&pnBlk, swrc.X + swrc.Width, swrc.Y, swrc.X + swrc.Width, swrc.Y + swrc.Height);
	gr->DrawLine(&pnWht, swrc.X, swrc.Y, swrc.X + swrc.Width, swrc.Y);
	gr->DrawLine(&pnWht, swrc.X, swrc.Y, swrc.X, swrc.Y + swrc.Height);
	delete gr;
}

void SlideSwitchWidget::Paint(DrawContext dc)
{
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
	char path[MAX_PATH];
	if (SynthFileExists(file))
		strcpy_s(path, MAX_PATH, file);
	else
		sprintf_s(path, MAX_PATH, "%s/%s", form->FormsDir(), file);
	return new Bitmap(WideChar(path));
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

	Bitmap *img = (Bitmap*)sipImg->GetImage(swOn ? 1 : 0);
	if (img)
	{
		int w = img->GetWidth();
		int h = img->GetHeight();
		int x = area.x + ((area.w - w) / 2);
		int y  = area.y + ((area.h - h) / 2);
		Graphics *gr = (Graphics *)dc;
		gr->DrawImage(img, x, y);
	}
}

void LampWidget::CreateImage()
{
	if (GetSwitchImage(lampSwitch))
		return;

	Rect back;
	back.X = 0;
	back.Y = 0;
	back.Width = area.w;
	back.Height = area.h;

	Rect baseRing;
	baseRing.X = 0;
	baseRing.Y = 0;
	int ring;
	if (area.w > area.h)
		ring = area.h - 1;
	else
		ring = area.w - 1;
	if (!(style & 2))
	{
		baseRing.Width = ring;
		baseRing.Height = ring;
	}
	else
	{
		baseRing.Width = area.w;
		baseRing.Height = area.h;
	}

	Rect lamp;
	lamp = baseRing;
	lamp.Inflate(-4, -4);

	Graphics *grOn = Graphics::FromImage((Bitmap*)sip->GetImage(1));
	Graphics *grOff = Graphics::FromImage((Bitmap*)sip->GetImage(0));
	grOn->SetSmoothingMode(SmoothingModeHighQuality);
	grOff->SetSmoothingMode(SmoothingModeHighQuality);

	LinearGradientBrush baseBrush(baseRing, Color(255,255,255), Color(96,96,96), 45.0);
	Pen olPen(Color(10,10,10), 0.0);
	SolidBrush backBrush(bgClr);
	grOn->FillRectangle(&backBrush, back);
	grOff->FillRectangle(&backBrush, back);
	if (style & 1)
	{
		grOn->FillRectangle(&baseBrush, baseRing);
		grOn->DrawRectangle(&olPen, baseRing);

		grOff->FillRectangle(&baseBrush, baseRing);
		grOff->DrawRectangle(&olPen, baseRing);
	}
	else
	{
		grOn->FillEllipse(&baseBrush, baseRing);
		grOn->DrawEllipse(&olPen, baseRing);

		grOff->FillEllipse(&baseBrush, baseRing);
		grOff->DrawEllipse(&olPen, baseRing);
	}

	LinearGradientBrush lampBrush(lamp, Color(128,128,128), Color(20,20,20), 45.0f);
	GraphicsPath path;
	if (style & 2)
	{
		if (style & 1)
			path.AddRectangle(back);
		else
			path.AddEllipse(back);
	}
	else
	{
		if (style & 1)
			path.AddRectangle(back);
		else
			path.AddEllipse(baseRing);
	}
	PathGradientBrush pthGrBrush(&path);
	pthGrBrush.SetCenterColor(hiColor);
	Color bounds(loColor);
	int count = 1;
	pthGrBrush.SetSurroundColors(&bounds, &count);
	if (style & 1)
	{
		grOn->FillRectangle(&pthGrBrush, lamp);
		grOn->DrawRectangle(&olPen, lamp);

		grOff->FillRectangle(&lampBrush, lamp);
		grOff->DrawRectangle(&olPen, lamp);
	}
	else
	{
		grOn->FillEllipse(&pthGrBrush, lamp);
		grOn->DrawEllipse(&olPen, lamp);

		grOff->FillEllipse(&lampBrush, lamp);
		grOff->DrawEllipse(&olPen, lamp);
	}

	delete grOn;
	delete grOff;
}

void LampWidget::Paint(DrawContext dc)
{
	SwitchWidget::Paint(dc);
}

void BoxWidget::Paint(DrawContext dc)
{
	if (!show)
		return;
	Graphics *gr = (Graphics*)dc;
	gr->SetSmoothingMode(SmoothingModeHighQuality);
	if (filled)
	{
		SolidBrush bk(bgClr);
		gr->FillRectangle(&bk, area.x, area.y, area.w, area.h);
	}
	if (style == 0)
	{
		Pen fgPen(fgClr, thick);
		gr->DrawRectangle(&fgPen, area.x, area.y, area.w, area.h);
	}
	else
	{
		Pen hiPen(hiClr, thick);
		Pen loPen(loClr, thick);
		Pen *p1, *p2;
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
			gr->DrawLine(p1, area.GetLeft(), area.GetTop(), area.GetRight(), area.GetTop());
			gr->DrawLine(p2, area.GetLeft(), area.GetBottom(), area.GetRight(), area.GetBottom());
		}
		if (area.h > 1)
		{
			gr->DrawLine(p1, area.GetLeft(), area.GetTop(), area.GetLeft(), area.GetBottom());
			gr->DrawLine(p2, area.GetRight(), area.GetTop(), area.GetRight(), area.GetBottom());
		}
	}
}

void TextWidget::Paint(DrawContext dc)
{
	Graphics *gr = (Graphics*)dc;
	if (text && show)
	{
		gr->SetSmoothingMode(SmoothingModeHighQuality);
		if (filled)
		{
			SolidBrush bck(bgClr);
			gr->FillRectangle(&bck, area.x, area.y, area.w, area.h);
		}

		FontFamily ff(WideChar(form->LabelFont()));
		StringFormat sf;
		if (align == 0) // left
		{
			sf.SetAlignment(StringAlignmentNear);
			sf.SetLineAlignment(StringAlignmentNear);
		}
		else if (align == 1) // center
		{
			sf.SetAlignment(StringAlignmentCenter);
			sf.SetLineAlignment(StringAlignmentCenter);
		}
		else if (align == 2) // right
		{
			sf.SetAlignment(StringAlignmentFar);
			sf.SetLineAlignment(StringAlignmentFar);
		}
		int style = 0;
		if (textBold)
			style += FontStyleBold;
		if (textItalic)
			style += FontStyleItalic;
		Font textFont(&ff, textHeight, style, UnitPixel);
		RectF sr;

		const wchar_t *wtext = WideChar(text);
		if (inset)
		{
			SolidBrush blk(Color::Black);
			SolidBrush wht(Color::White);
			sr.X = area.x - inset;
			sr.Y = area.y - inset;
			sr.Width = area.w;
			sr.Height = area.h;
			gr->DrawString(wtext, -1, &textFont, sr, &sf, &blk);
			sr.X = area.x + inset;
			sr.Y = area.y + inset;
			gr->DrawString(wtext, -1, &textFont, sr, &sf, &wht);
		}
		else if (shadow)
		{
			SolidBrush blk(Color::Black);
			sr.X = area.x + shadow;
			sr.Y = area.y + shadow;
			sr.Width = area.w;
			sr.Height = area.h;
			gr->DrawString(wtext, -1, &textFont, sr, &sf, &blk);
		}
		SolidBrush br(fgClr);
		sr.X = area.x;
		sr.Y = area.y;
		sr.Width = area.w;
		sr.Height = area.h;
		gr->DrawString(wtext, -1, &textFont, sr, &sf, &br);
	}
	if (focus)
	{
		Pen olPen(fgClr, 2.0);
		Rect frc;
		frc.X = area.x;
		frc.Y = area.y;
		frc.Width = area.w;
		frc.Height = area.h;
		frc.Inflate(-1,-1);
		gr->DrawRectangle(&olPen, frc);
	}
}

void GraphWidget::Paint(DrawContext dc)
{
	if (!show)
		return;

	Graphics *gr = (Graphics*)dc;
	gr->SetSmoothingMode(SmoothingModeHighQuality);
	SolidBrush bk(bgClr);
	gr->FillRectangle(&bk, area.x, area.y, area.w, area.h);
	Pen pn(fgClr, 1);
	gr->DrawRectangle(&pn, area.x, area.y, area.w, area.h);

	if (numVals < 1 || vals == 0)
		return;

	float scale = (float) (area.h - 2) / range;
	float incr = (float) (area.w - 2) / (float) numVals;
	if (bar)
	{
		SolidBrush br(grClr);
		RectF r;
		r.X = area.x + 1;
		r.Width = incr * barWidth;
		for (int n = 0; n < numVals; n++)
		{
			float y = vals[n] * scale;
			r.Y = area.GetBottom() - 1 - y;
			r.Height = y;
			gr->FillRectangle(&br, r);
			r.X += incr;
		}
	}
	else
	{
		Pen gpn(grClr, thick);
		PointF pt[2];
		pt[0].X = area.x + 1;
		pt[0].Y = area.GetBottom();
		for (int n = 0; n < numVals; n++)
		{
			float y = vals[n] * scale;
			pt[1].X = pt[0].X + incr;
			pt[1].Y = area.GetBottom() - 1 - y;
			gr->DrawLine(&gpn, pt[0], pt[1]);
			pt[0] = pt[1];
		}
	}
}

void EnvelopeWidget::Paint(DrawContext dc)
{
	Graphics *gr = (Graphics*)dc;
	gr->SetSmoothingMode(SmoothingModeHighQuality);
	SolidBrush bck(bgClr);
	gr->FillRectangle(&bck, area.x, area.y, area.w, area.h);
	if (border)
	{
		Pen fr(frClr, border);
		gr->DrawRectangle(&fr, area.x, area.y, area.w, area.h);
	}
	if (numSegs < 1 || vals == 0)
		return;
	int num = numSegs;

	Pen pn(fgClr, 1);
	int ndx;

	float lvlscale = (float) (area.h - 2) / ampRange;
	float rtscale = (float) (area.w - 2) / tmRange;

	float rtval = 0;
	float lvlval = 0;
	PointF pt1;
	PointF pt2;
	REAL xo = (REAL) area.GetLeft() + 1;
	REAL yo = (REAL) area.GetBottom() - 1;
	pt1.X = xo;
	pt1.Y = yo - (start * lvlscale);
	pt2.X = pt1.X;
	pt2.Y = pt1.Y;
	if (susOn)
		num--;
	for (ndx = 0; ndx < num; ndx++)
	{
		rtval += vals[ndx].rate;
		if (rtval > tmRange)
			rtval = tmRange;
		lvlval = vals[ndx].level;
		if (lvlval > ampRange)
			lvlval = ampRange;
		pt2.X = xo + (rtval * rtscale);
		pt2.Y = yo - (lvlval * lvlscale);
		gr->DrawLine(&pn, pt1, pt2);
		pt1.X = pt2.X;
		pt1.Y = pt2.Y;
	}
	if (susOn)
	{
		pt2.X = (REAL) area.GetRight() - (vals[ndx].rate * rtscale);
		if (pt2.X > pt1.X)
		{
			gr->DrawLine(&pn, pt1, pt2);
			pt1.X = pt2.X;
			pt2.X = (REAL) area.GetRight();
		}
		lvlval = vals[ndx].level;
		if (lvlval > ampRange)
			lvlval = ampRange;
		pt2.Y = yo - (lvlval * lvlscale);
		gr->DrawLine(&pn, pt1, pt2);
		pt1.X = pt2.X;
		pt1.Y = pt2.Y;
	}
	else if (pt1.Y != yo)
	{
		pt2.X = pt1.X;
		pt2.Y = yo;
		gr->DrawLine(&pn, pt1, pt2);
	}
}

void WaveWidget::Paint(DrawContext dc)
{
	Graphics *gr = (Graphics*)dc;
	gr->SetSmoothingMode(SmoothingModeHighQuality);
	SolidBrush bck(bgClr);
	gr->FillRectangle(&bck, area.x, area.y, area.w, area.h);
	if (border)
	{
		Pen pn(frClr, border);
		gr->DrawRectangle(&pn, area.x, area.y, area.w, area.h);
	}
	if (wvType == 1)
		PlotSum(gr);
	else if (wvType == 2)
		PlotSeg(gr);
}

void WaveWidget::PlotSum(DrawContext dc)
{
	Graphics *gr = (Graphics *)dc;
	gr->SetSmoothingMode(SmoothingModeHighQuality);
	int cx = area.w - 4;
	int cy = area.h - 4;

	if (cx > 0 && cy > 0)
	{
		int parts = 0;
		int x, n;
		float mid = (float)cy / 2.0;
		float maxmul = 1;
		float pp[WFI_MAXPART];
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

		double phsIncr = twoPI / (double) cx;
		float *ampvals = new float[cx];
		float maxAmp = 0.0;
		float sigma;
		float sigK = PI / maxmul;
		float sigN;
		for (x = 0; x < cx; x++)
		{
			ampvals[x] = 0.0;
			for (n = 0; n < WFI_MAXPART; n++)
			{
				if (!on[n])
					continue;
				if (gibbs && n > 0)
				{
					sigN = (float)n * sigK;
					sigma = amps[n] * (sin(sigN) / sigN);
				}
				else
					sigma = amps[n];
				ampvals[x] += (float) sin(pp[n]) * sigma;
				pp[n] += phsIncr * (double) (n+1);
				if (pp[n] >= twoPI)
					pp[n] -= twoPI;
			}
			if (fabs(ampvals[x]) > maxAmp)
				maxAmp = fabs(ampvals[x]);
		}
		PointF pt1;
		PointF pt2;
		REAL xo = (REAL) area.x + 2;
		REAL yo = (REAL) area.y + 2 + mid;
		Pen pn(fgClr, 1);
		Pen fr(frClr, 1);
		gr->DrawLine(&fr, xo, yo, (REAL) area.GetRight() - 2, yo);
		pt1.X = xo;
		pt1.Y = yo - ((ampvals[0] / maxAmp) * mid);
		for (x = 1; x < cx; x++)
		{
			pt2.X = pt1.X + 1;
			pt2.Y = yo - ((ampvals[x] / maxAmp) * mid);
			gr->DrawLine(&pn, pt1, pt2);
			pt1.X = pt2.X;
			pt1.Y = pt2.Y;
		}
		pt2.X = pt1.X + 1;
		pt2.Y = yo - ((ampvals[0] / maxAmp) * mid);
		gr->DrawLine(&pn, pt1, pt2);
		delete ampvals;
	}
}

void WaveWidget::PlotSeg(DrawContext dc)
{
	Graphics *gr = (Graphics*)dc;

	int cx = area.w - 4;
	int cy = area.h - 4;

	if (cx > 0 && cy > 0)
	{
		int n;
		REAL cxF = (REAL) cx;
		REAL cyF = (REAL) cy;
		REAL top = (REAL) area.y + 2;
		REAL bot = top + cyF;
		REAL mid = cyF / 2.0;
		REAL rgt = (REAL) (area.x + 2) + cxF;
		REAL yo = (REAL) area.y + 2 + mid;
		PointF pt1;
		PointF pt2;
		pt1.X = (REAL) area.x + 2;
		pt1.Y = yo;
		pt2.X = rgt;
		pt2.Y = yo;
		Pen pn(fgClr, 1);
		Pen fr(frClr, 1);
		gr->DrawLine(&fr, pt1, pt2); // center line
		for (n = 0; n < WFI_MAXPART; n++)
		{
			if (!on[n])
				continue;
			pt2.X = pt1.X + (phs[n] * cxF);
			if (pt2.X > rgt)
				pt2.X = rgt;
			pt2.Y = yo - (amps[n] * mid);
			if (pt2.Y < top)
				pt2.Y = top;
			else if (pt2.Y > bot)
				pt2.Y = bot;
			gr->DrawLine(&pn, pt1, pt2);
			pt1.X = pt2.X;
			pt1.Y = pt2.Y;
		}
	}
}

void SliderWidget::CreateImage()
{
}

void SliderWidget::Paint(DrawContext dc)
{
	Graphics *gr = (Graphics *)dc;
	gr->SetSmoothingMode(SmoothingModeHighQuality);

	Rect bgr;
	bgr.X = area.x;
	bgr.Y = area.y;
	bgr.Width = area.w;
	bgr.Height = area.h;
	Pen pnBlk(Color(40,40,40), 1.0f);
	Pen pnWht(Color(220,220,220), 1.0f);
	Pen pnTrk(Color(10,20,20), 3.0f);
	pnWht.SetStartCap(LineCapTriangle);
	pnWht.SetEndCap(LineCapTriangle);
	pnBlk.SetStartCap(LineCapTriangle);
	pnBlk.SetEndCap(LineCapTriangle);
	LinearGradientBrush backBrush(bgr, Color(120,120,120), Color(80,80,80), 60.0f);
	gr->FillRectangle(&backBrush, bgr);
	gr->DrawLine(&pnBlk, bgr.X, bgr.GetBottom()-1, bgr.GetRight()-1, bgr.GetBottom()-1); //bottom
	gr->DrawLine(&pnBlk, bgr.GetRight()-1, bgr.Y, bgr.GetRight()-1, bgr.GetBottom()-1);  //right
	gr->DrawLine(&pnWht, bgr.X, bgr.Y, bgr.GetRight()-1, bgr.Y); // top
	gr->DrawLine(&pnWht, bgr.X, bgr.Y, bgr.X, bgr.GetBottom()); // left

	SolidBrush trackBrush(Color(10,10,10));
//	bgr.X = slideTrack.x - area.x;
//	bgr.Y = slideTrack.y - area.y;
//	bgr.Width = slideTrack.w;
//	bgr.Height = slideTrack.h;

	if (vertical)
		gr->DrawLine(&pnTrk, slideTrack.x, slideTrack.y, slideTrack.x, slideTrack.GetBottom());
	else
		gr->DrawLine(&pnTrk, slideTrack.x, slideTrack.y, slideTrack.GetRight(), slideTrack.y);

	HatchBrush knobBrush(vertical ? HatchStyleNarrowHorizontal : HatchStyleNarrowVertical,
		                 Color(100,100,100), Color(40, 40, 40));
	bgr.X = knobPos.x;
	bgr.Y = knobPos.y;
	bgr.Width = knobPos.w;
	bgr.Height = knobPos.h;
	gr->FillRectangle(&knobBrush, bgr);
	gr->DrawRectangle(&pnBlk, bgr);
	if (vertical)
		gr->DrawLine(&pnWht, bgr.X, bgr.Y + (bgr.Height/2), bgr.GetRight(), bgr.Y+(bgr.Height/2));
	else
		gr->DrawLine(&pnWht, bgr.X + bgr.Width/2, bgr.Y, bgr.X + bgr.Width/2, bgr.GetBottom());
}