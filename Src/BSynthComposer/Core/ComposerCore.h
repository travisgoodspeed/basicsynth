//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef COMPOSER_CORE_H
#define COMPOSER_CORE_H

#ifndef MAX_PATH
#define MAX_PATH 512
#endif
#ifdef _WIN32
# ifndef snprintf
#  define snprintf _snprintf
# endif
#endif

#include "ProjectOptions.h"
#include "WindowTypes.h"
#include "ProjectItem.h"
#include "SynthWidget.h"
#include "KnobWidget.h"
#include "SliderWidget.h"
#include "SwitchWidget.h"
#include "TextWidget.h"
#include "EnvelopeWidget.h"
#include "WaveWidget.h"
#include "KeyboardWidget.h"
#include "WidgetForm.h"
#include "KeyboardForm.h"
#include "SynthEdit.h"

#endif
