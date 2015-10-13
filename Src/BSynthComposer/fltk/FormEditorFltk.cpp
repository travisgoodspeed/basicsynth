//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Form edit window implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "FormEditorFltk.h"
#include "MainFrm.h"

void HScrollCB(Fl_Widget *wdg, void *arg)
{
	FormEditorFltk* ed = (FormEditorFltk*)wdg->parent();
	ed->OnScroll();
}

void VScrollCB(Fl_Widget *wdg, void *arg)
{
	FormEditorFltk* ed = (FormEditorFltk*)wdg->parent();
	ed->OnScroll();
}

FormEditorFltk::FormEditorFltk(int X, int Y, int W, int H) : Fl_Group(X, Y, W, H, 0)
{
	SynthWidget::colorMap.Find("bg", bgColor);
	SynthWidget::colorMap.Find("fg", fgColor);
	//printf("FormEditorFltk %d %d %d %d\n", x, y, w, h);
	box(FL_FLAT_BOX);
	color(0x80808000);
	setCapture = 0;
	item = 0;
	vscrl = new Fl_Scrollbar(0,0,0,0);
	vscrl->type(FL_VERTICAL);
	vscrl->callback(VScrollCB, this);
	hscrl = new Fl_Scrollbar(0,0,0,0);
	hscrl->type(FL_HORIZONTAL);
	hscrl->callback(HScrollCB, this);
	end();
	resizable(0);
}

FormEditorFltk::~FormEditorFltk()
{
//	printf("Editor destroyed.\n");
	delete form;
}


int FormEditorFltk::handle(int e)
{
	int mx = Fl::event_x();
	int my = Fl::event_y();
//	printf("FormEditorFltk handle(%d) at [%d,%d]\n", e, mx, my);
	if (hscrl && hscrl->visible())
	{
		if (mx >= hscrl->x() && mx <= hscrl->x() + hscrl->w()
		 && my >= hscrl->y() && my <= hscrl->y() + hscrl->h())
		{
			//printf("Pass to hscroll\n");
			if (hscrl->handle(e) && e == FL_PUSH)
				Fl::pushed(hscrl);
			return 0;
		}
	}
	if (vscrl && vscrl->visible())
	{
		if (mx >= vscrl->x() && mx <= vscrl->x() + vscrl->w()
		 && my >= vscrl->y() && my <= vscrl->y() + vscrl->h())
		{
			//printf("Pass to vscroll\n");
			if (vscrl->handle(e) && e == FL_PUSH)
				Fl::pushed(vscrl);
			return 0;
		}
	}

	if (form && (e == FL_PUSH || e == FL_DRAG || e == FL_RELEASE))
	{
		setCapture = 0;
		switch (e)
		{
		case FL_PUSH:
			form->BtnDn(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		case FL_DRAG:
			form->MouseMove(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		case FL_RELEASE:
			form->BtnUp(mx, my, Fl::event_shift(), Fl::event_ctrl());
			break;
		}
		//return setCapture;
		return 1;
	}
	return 0;
}

void FormEditorFltk::resize(int X, int Y, int W, int H)
{
//	printf("FormEditor: resize(%d,%d,%d,%d)\n", X, Y, W,H);
	Fl_Widget::resize(X, Y, W, H);
	if (form)
	{
		form->MoveTo(X, Y);
		Resize();
	}
}

void FormEditorFltk::SetItem(ProjectItem *p)
{
	if (item != NULL)
	{
		item->SetEditor(0);
		item->Release();
	}
	if ((item = p) != NULL)
	{
		item->SetEditor(this);
		item->AddRef();
		//label(p->GetName());
		form = p->CreateForm(x(), y());
		if (form)
		{
			form->SetFormEditor(this);
			form->GetParams();
		}
		Resize();
	}
}

void FormEditorFltk::Resize()
{
	int scroll_width = 16;//Fl::scrollbar_size();
	int cx = w();
	int cy = h();
	if (form)
		form->GetSize(cx, cy);
	//printf("Form resize [%d,%d,%d,%d] form is [%d,%d]\n", X, Y, W, H, cx, cy);
	int hon = cx > w();
	int von = cy > h();
	if (von && cx > (w()-scroll_width))
		hon = 1;
	if (hon && cy > (h()-scroll_width))
		von = 1;
	//printf("   hon = %d, von = %d\n", hon, von);
	if (hon)
	{
		int hw = w();
		double hr = (double)(cx - w());
		if (von)
		{
			hw -= scroll_width;
			hr += scroll_width;
		}
		hscrl->resize(x(), y()+h()-scroll_width, hw, scroll_width);
		//printf("Set hscroll to %f\n", hr);
		hscrl->Fl_Slider::value(0.0);
		hscrl->range(0.0, hr);
		hscrl->slider_size((double)hw / (double) cx);
		hscrl->show();
	}
	else
		hscrl->hide();
	if (von)
	{
		int vh = h();
		double vr = (double)(cy - h());
		if (hon)
		{
			vh -= scroll_width;
			vr += scroll_width;
		}
		vscrl->resize(x()+w()-scroll_width, y(), scroll_width, vh);
		//printf("Set vscroll to %f\n", vr);
		vscrl->Fl_Slider::value(0.0);
		vscrl->range(0.0, vr);
		vscrl->slider_size((double)vh / (double)cy);
		vscrl->show();
	}
	else
		vscrl->hide();
	redraw();
}

void FormEditorFltk::draw()
{
//	Fl_Group::draw();
	Fl_Widget *const*childList = array();
	if (damage() == FL_DAMAGE_CHILD)
	{
		// now draw all the children atop the background:
		for (int i = children(); i--; childList++)
			update_child(**childList);
	}
	else
	{
		int fw = w();
		int fh = h();
		int offs[2] = {0,0};
		int hon = hscrl->visible();
		int von = vscrl->visible();
		if (hon)
			fh -= hscrl->h();
		if (von)
			fw -= vscrl->w();

		DrawContext ctx = (DrawContext)&offs[0];
		//printf("FormEditor: draw(%d,%d,%d,%d) hon=%d, von=%d\n", x(), y(), fw, fh, hon, von);
		if (form)
		{
			int part = damage() == FL_DAMAGE_USER1;
			if (!part)
				fl_push_clip(x(), y(), fw, fh);
			//fl_color((int) (bgColor >> 16) & 0xff, (int) (bgColor >> 8) & 0xff, (int) bgColor & 0xff);
			fl_color((Fl_Color)(bgColor << 8));
			fl_rectf(x(), y(), fw, fh);
			form->RedrawForm(ctx);
			if (!part)
				fl_pop_clip();
		}
		// now draw all the children atop the background:
		for (int i = children(); i--; childList++)
			draw_child(**childList);
		if (hon && von)
			fl_draw_box(FL_UP_BOX, x()+fw, y()+fh, w()-fw, h()-fh, Fl_Color(0xa0a0a000));
	}
}

void FormEditorFltk::CopyToClipboard()
{
}

void FormEditorFltk::PasteFromClipboard()
{
}

long FormEditorFltk::EditState()
{
	return 0;
}

void FormEditorFltk::Capture()
{
	setCapture = 1;
}

void FormEditorFltk::Release()
{
	setCapture = 0;
}

void FormEditorFltk::SetForm(WidgetForm *wf)
{
	form = wf;
	wf->SetFormEditor(this);
	form->MoveTo(x(), y());
}

void FormEditorFltk::Redraw(SynthWidget *wdg)
{
	if (wdg)
	{
		wdgRect area = wdg->GetArea();
		damage(FL_DAMAGE_USER1, area.x, area.y, area.w, area.h);
		SynthWidget *bud = wdg->GetBuddy2();
		while (bud)
		{
			area = bud->GetArea();
			damage(FL_DAMAGE_USER1, area.x, area.y, area.w, area.h);
			bud = bud->GetBuddy2();
		}
	}
	else
		redraw();
}

void FormEditorFltk::DrawWidget(SynthWidget *wdg)
{
	int left = x();
	int right = left + w();
	int top = y();
	int bottom = top + h();
	int cy, cx;
	int offs[2] = {0, 0};

	if (hscrl->visible())
		bottom = hscrl->y();

	if (vscrl->visible())
		right = vscrl->x();

	wdgRect rc = wdg->GetArea();
	cx = rc.GetRight();
	cy = rc.GetBottom();
	if (rc.x > right || rc.y > bottom || cx < left || cy < top)
		return;

	if (rc.x < 0)
		left = 0;
	else if (rc.x > left)
		left = rc.x;
	if (rc.y < 0)
		top = 0;
	else if (rc.y > top)
		top = rc.y;
	if (cx < right)
		right = cx;
	if (cy < bottom)
		bottom = cy;
	//printf("DrawWidget %d, %d, %d, %d\n", left, top, right, bottom);
	if (right > left && bottom > top)
	{
		fl_clip_region(XRectangleRegion(left, top, right - left, bottom - top));
		wdg->Paint((DrawContext)offs);
	}
}

void FormEditorFltk::OnScroll()
{
	double hv = hscrl->value();
	double vv = vscrl->value();
	int xpos = x() - (int) hv;
	int ypos = y() - (int) vv;
	//printf("FormEditorFltk: Scroll hv=%f, vv=%f, x=%d, y=%d, fx=%d, fy=%d\n", hv, vv, xpos, ypos, x(), y());
	form->MoveTo(xpos, ypos);
	redraw();
}
