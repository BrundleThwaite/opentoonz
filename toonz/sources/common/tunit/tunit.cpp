

#include "tunit.h"
#include "tconvert.h"
#include <math.h>

// Note: convertTo() means: convert from standard unit TO specific unit
// e.g. if unit=cm (standard=inch) then convertTo(1) == 2.54
//-------------------------------------------------------------------

/*
class VerticalCameraFldUnitConverter : public TUnitConverter {
public:
  static double m_factor;
  VerticalCameraFldUnitConverter() {}
  TUnitConverter * clone() const {return new VerticalCameraFldUnitConverter(*this);}
  double convertTo(double v) const {return v*m_factor;}
  double convertFrom(double v) const {return v/m_factor;}
};

double VerticalCameraFldUnitConverter::m_factor = 1;
*/

namespace UnitParameters
{

std::pair<double, double> dummyCurrentDpiGetter() { return std::make_pair<double, double>(72, 72); }

CurrentDpiGetter currentDpiGetter = &dummyCurrentDpiGetter;

void setCurrentDpiGetter(CurrentDpiGetter f) { currentDpiGetter = f; }
}

//-------------------------------------------------------------------

class VerticalFldUnitConverter : public TUnitConverter
{
	double m_factor;

public:
	static double m_fieldGuideAspectRatio;

	VerticalFldUnitConverter(double factor) : m_factor(factor) {}
	TUnitConverter *clone() const { return new VerticalFldUnitConverter(*this); }
	double convertTo(double v) const { return v * m_fieldGuideAspectRatio * m_factor; }
	double convertFrom(double v) const { return v / (m_fieldGuideAspectRatio * m_factor); }
};
double VerticalFldUnitConverter::m_fieldGuideAspectRatio = 1.38;

//-------------------------------------------------------------------

namespace UnitParameters
{

void setFieldGuideAspectRatio(double ar)
{
	assert(ar > 0);
	VerticalFldUnitConverter::m_fieldGuideAspectRatio = ar;
}

double getFieldGuideAspectRatio()
{
	return VerticalFldUnitConverter::m_fieldGuideAspectRatio;
}
}

//===================================================================

class TangentConverter : public TUnitConverter
{
public:
	TangentConverter() {}
	TUnitConverter *clone() const { return new TangentConverter(*this); }
	double convertTo(double v) const { return 180.0 * atan(v) / TConsts::pi; }
	double convertFrom(double v) const { return tan(TConsts::pi * v / 180.0); }
};

//===================================================================

class TPixelUnitXConverter : public TUnitConverter
{
public:
	TPixelUnitXConverter() {}
	TUnitConverter *clone() const { return new TPixelUnitXConverter(*this); }
	double convertTo(double v) const { return v * UnitParameters::currentDpiGetter().first; }
	double convertFrom(double v) const { return v / UnitParameters::currentDpiGetter().first; }
};

class TPixelUnitYConverter : public TUnitConverter
{
public:
	TPixelUnitYConverter() {}
	TUnitConverter *clone() const { return new TPixelUnitYConverter(*this); }
	double convertTo(double v) const { return v * UnitParameters::currentDpiGetter().second; }
	double convertFrom(double v) const { return v / UnitParameters::currentDpiGetter().second; }
};

//===================================================================

TUnit::TUnit(wstring ext, TUnitConverter *converter)
	: m_defaultExtension(ext), m_converter(converter)
{
	m_extensions.push_back(ext);
	if (m_converter == 0)
		m_converter = new TSimpleUnitConverter();
}

//-------------------------------------------------------------------

TUnit::TUnit(const TUnit &src)
	: m_defaultExtension(src.m_defaultExtension), m_extensions(src.m_extensions), m_converter(src.m_converter->clone())
{
}

//-------------------------------------------------------------------

TUnit::~TUnit()
{
	delete m_converter;
}

//-------------------------------------------------------------------

void TUnit::addExtension(wstring ext)
{
	if (std::find(
			m_extensions.begin(), m_extensions.end(), ext) == m_extensions.end())
		m_extensions.push_back(ext);
	if (m_defaultExtension.empty())
		m_defaultExtension = ext;
}

//-------------------------------------------------------------------

bool TUnit::isExtension(wstring ext) const
{
	return std::find(
			   m_extensions.begin(), m_extensions.end(), ext) != m_extensions.end();
}

//-------------------------------------------------------------------

void TUnit::setDefaultExtension(wstring ext)
{
	if (!ext.empty() && std::find(m_extensions.begin(), m_extensions.end(), ext) == m_extensions.end())
		m_extensions.push_back(ext);
	m_defaultExtension = ext;
}

//===================================================================

TMeasure::TMeasure(string name, TUnit *mainUnit)
	: m_name(name), m_mainUnit(0), m_standardUnit(0), m_currentUnit(0), m_defaultValue(0)
{
	add(mainUnit);
	m_mainUnit = m_currentUnit = m_standardUnit = mainUnit;
}

//-------------------------------------------------------------------

TMeasure::TMeasure(const TMeasure &src)
	: m_name(src.m_name), m_mainUnit(src.m_mainUnit), m_currentUnit(src.m_currentUnit), m_standardUnit(src.m_standardUnit), m_defaultValue(src.m_defaultValue)
{
	std::map<wstring, TUnit *>::const_iterator it;
	for (it = src.m_extensions.begin();
		 it != src.m_extensions.end(); ++it) {
		TUnit *u = it->second;
		assert(u);
		const std::vector<wstring> &e = u->getExtensions();
		assert(std::find(e.begin(), e.end(), it->first) != e.end());
		m_extensions[it->first] = u;
	}
}

//-------------------------------------------------------------------

TMeasure::~TMeasure()
{
}

//-------------------------------------------------------------------

void TMeasure::add(TUnit *unit)
{
	const std::vector<wstring> &e = unit->getExtensions();
	for (int i = 0; i < (int)e.size(); i++) {
		wstring ext = e[i];
		assert(m_extensions.count(ext) == 0);
		m_extensions[ext] = unit;
	}
}

//-------------------------------------------------------------------

TUnit *TMeasure::getUnit(wstring ext) const
{
	std::map<wstring, TUnit *>::const_iterator it;
	it = m_extensions.find(ext);
	return it == m_extensions.end() ? 0 : it->second;
}

//-------------------------------------------------------------------

void TMeasure::setCurrentUnit(TUnit *unit)
{
	assert(unit);
	assert(m_extensions.count(unit->getDefaultExtension()) > 0);
	m_currentUnit = unit;
}

//-------------------------------------------------------------------

void TMeasure::setStandardUnit(TUnit *unit)
{
	assert(unit);
	assert(m_extensions.count(unit->getDefaultExtension()) > 0);
	m_standardUnit = unit;
}

//===================================================================

TMeasureManager::TMeasureManager()
{
	TUnit
		inch(L"in"),
		cm(L"cm", new TSimpleUnitConverter(2.54)),
		mm(L"mm", new TSimpleUnitConverter(25.4)),

		xfld(L"fld", new TSimpleUnitConverter(2)),
		cameraXFld(L"fld", new TSimpleUnitConverter(1)),

		yfld(L"fld", new VerticalFldUnitConverter(2)),
		cameraYFld(L"fld", new VerticalFldUnitConverter(1)),

		levelXFld(L"fld", new TSimpleUnitConverter(1)),
		levelYFld(L"fld", new VerticalFldUnitConverter(1)),

		internalZDepth(L"internal.zdepth"),
		zdepth(L"zdepth", new TSimpleUnitConverter(1)),
		degree(L"\u00b0"),
		scale(L"*"),
		percentage2(L"%"),
		shear(L"sh"),
		shearAngle(L"\u00b0", new TangentConverter()),
		percentage(L"%", new TSimpleUnitConverter(100)),
		colorChannel(L"", new TSimpleUnitConverter(255)),
		dummy(L""),
		xPixel(L"px", new TPixelUnitXConverter()),
		yPixel(L"px", new TPixelUnitYConverter());

	inch.addExtension(L"inch");
	inch.addExtension(L"\"");
	inch.addExtension(L"''");
	inch.setDefaultExtension(L"\"");

	xPixel.addExtension(L"pixel");
	yPixel.addExtension(L"pixel");

	xfld.addExtension(L"field");
	cameraXFld.addExtension(L"field");

	yfld.addExtension(L"field");
	cameraYFld.addExtension(L"field");

	levelXFld.addExtension(L"field");
	levelYFld.addExtension(L"field");

	xfld.addExtension(L"F");
	yfld.addExtension(L"F");
	cameraXFld.addExtension(L"F");
	cameraYFld.addExtension(L"F");

	levelXFld.addExtension(L"F");
	levelYFld.addExtension(L"F");

	TMeasure *length, *m;

	length = m = new TMeasure("length", inch.clone());
	m->add(cm.clone());
	/*---
	Fxの寸法パラメータは単位なし（実際にはStageInch（1 StageInch = 1/53.33333 inch）という値）
	Fxの寸法パラメータからExpressionで単位のあるパラメータを参照すると、
	カレントUnitによってFxの計算結果が変わってしまう。
	tcomposerで用いられるカレントUnitはデフォルト値なので、
	ここでデフォルトのカレントUnitをmmにしておくことで、
	Unit = mm でシーンを作っておけば、作業時と同じRender結果が得られるようにする。
	---*/
	TUnit *mmUnit = mm.clone();
	m->add(mmUnit);
	m->setCurrentUnit(mmUnit);
	add(m);

	m = new TMeasure(*length);
	m->setName("length.x");
	m->add(xfld.clone());
	m->add(xPixel.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("length.y");
	m->add(yfld.clone());
	m->add(yPixel.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("length.lx");
	m->add(xfld.clone());
	m->add(xPixel.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("length.ly");
	m->add(yfld.clone());
	m->add(yPixel.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("camera.lx");
	m->add(cameraXFld.clone());
	m->add(xPixel.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("camera.ly");
	m->add(cameraYFld.clone());
	m->add(yPixel.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("level.lx");
	m->add(levelXFld.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("level.ly");
	m->add(levelYFld.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("canvas.lx");
	m->add(cameraXFld.clone());
	add(m);

	m = new TMeasure(*length);
	m->setName("canvas.ly");
	m->add(cameraYFld.clone());
	add(m);

	TUnit fxLength(L"fxLength"),
		fxInch(L"in", new TSimpleUnitConverter(1 / 53.33333)),
		fxCm(L"cm", new TSimpleUnitConverter(2.54 / 53.33333)),
		fxMm(L"mm", new TSimpleUnitConverter(25.4 / 53.33333)),
		fxXfld(L"fld", new TSimpleUnitConverter(2 / 53.33333));
	fxInch.addExtension(L"inch");
	fxInch.addExtension(L"\"");
	fxInch.addExtension(L"''");
	fxInch.setDefaultExtension(L"\"");
	fxXfld.addExtension(L"field");
	fxXfld.addExtension(L"F");
	m = new TMeasure("fxLength", fxLength.clone());
	m->add(fxInch.clone());
	m->add(fxCm.clone());
	m->add(fxMm.clone());
	m->add(fxXfld.clone());
	add(m);

	m = new TMeasure("angle", degree.clone());
	add(m);

	m = new TMeasure("scale", scale.clone());
	TUnit *unit = percentage.clone();
	m->add(unit);
	m->setCurrentUnit(unit);
	m->setStandardUnit(unit);
	add(m);

	m = new TMeasure("percentage", scale.clone());
	unit = percentage.clone();
	m->add(unit);
	m->setCurrentUnit(unit);
	m->setStandardUnit(unit);
	add(m);

	m = new TMeasure("percentage2", percentage2.clone());
	add(m);

	m = new TMeasure("shear", shear.clone());
	unit = shearAngle.clone();
	m->add(unit);
	m->setCurrentUnit(unit);
	m->setStandardUnit(unit);
	add(m);

	m = new TMeasure("colorChannel", colorChannel.clone());
	add(m);

	/*
  m = new TMeasure("zdepth", internalZDepth.clone());
  unit = zdepth.clone();
  m->add(unit);
  m->setCurrentUnit(unit);
  m->setStandardUnit(unit);
  add(m);
*/
	m = new TMeasure("dummy", dummy.clone());
	add(m);
}

//-------------------------------------------------------------------

void TMeasureManager::add(TMeasure *m)
{
	m_measures[m->getName()] = m;
}

//-------------------------------------------------------------------

TMeasure *TMeasureManager::get(string name) const
{
	std::map<string, TMeasure *>::const_iterator it;
	it = m_measures.find(name);
	if (it == m_measures.end())
		return 0;
	else
		return it->second;
}

//===================================================================

TMeasuredValue::TMeasuredValue(string measureName)
	: m_measure(0), m_value(0)
{
	setMeasure(measureName);
}

//-------------------------------------------------------------------

TMeasuredValue::~TMeasuredValue()
{
}

//-------------------------------------------------------------------

void TMeasuredValue::setMeasure(const TMeasure *measure)
{
	assert(measure);
	if (!measure)
		return;
	m_measure = measure;
	m_value = m_measure->getDefaultValue();
}

//-------------------------------------------------------------------

void TMeasuredValue::setMeasure(string measureName)
{
	setMeasure(TMeasureManager::instance()->get(measureName));
}

//-------------------------------------------------------------------

bool TMeasuredValue::setValue(wstring s, int *pErr)
{
	if (s == ::toWideString("")) {
		if (pErr)
			*pErr = -1;
		return false;
	}
	const TUnit *unit = m_measure->getCurrentUnit();
	double value = 0;
	bool valueFlag = false;
	int i = 0, len = s.length();
	// skip blanks
	i = s.find_first_not_of(::toWideString(" \t"));
	assert(i != (int)wstring::npos);
	int j = i;
	// match number
	if (i < len && (s[i] == L'-' || s[i] == L'+'))
		i++;
	while (i < len && L'0' <= s[i] && s[i] <= L'9')
		i++;
	if (i < len && s[i] == L'.') {
		i++;
		while (i < len && L'0' <= s[i] && s[i] <= L'9')
			i++;
		if (i < len && (s[i] == L'E' || s[i] == L'e')) {
			i++;
			if (i < len && (s[i] == L'-' || s[i] == L'+'))
				i++;
			while (i < len && L'0' <= s[i] && s[i] <= L'9')
				i++;
		}
	}
	if (i > j) {
		value = toDouble(s.substr(j, i - j));
		valueFlag = true;
		// skip blanks
		i = s.find_first_not_of(::toWideString(" \t"), i);
		if (i == (int)wstring::npos)
			i = s.length();
	}

	// remove trailing blanks
	if (i < (int)s.length()) {
		j = i;
		i = s.find_last_not_of(::toWideString(" \t"));
		if (i == (int)wstring::npos)
			i = len - 1;
		if (j <= i) {
			wstring unitExt = s.substr(j, i + 1 - j);
			unit = m_measure->getUnit(unitExt);
			if (!unit) {
				if (pErr)
					*pErr = -2;
				return false;
			}
		}
	}

	if (valueFlag) {
		double newValue = unit->convertFrom(value);
		if (m_value == newValue) {
			if (pErr)
				*pErr = 0;
			return false;
		}
		m_value = newValue;
	}
	if (pErr)
		*pErr = (valueFlag || unit != 0) ? 0 : -3;
	return valueFlag || unit != 0;
}

//-------------------------------------------------------------------

wstring TMeasuredValue::toWideString(int decimals) const
{
	double v = getValue(CurrentUnit);
	string s = toString(v, decimals);
	if (s.find('.') != string::npos) {
		int i = s.length();
		while (i > 0 && s[i - 1] == '0')
			i--;
		if (i > 0 && s[i - 1] == '.')
			i--;
		if (i < (int)s.length())
			s = s.substr(0, i);
	}
	wstring measure = m_measure->getCurrentUnit()->getDefaultExtension();
	if (measure.empty())
		return ::toWideString(s);
	return ::toWideString(s) + ::toWideString(" ") + measure;
}

//===================================================================

namespace
{

class ZDepthUnitConverter : public TUnitConverter
{
	TMeasureManager::CameraSizeProvider *m_cameraSizeProvider;

public:
	ZDepthUnitConverter(TMeasureManager::CameraSizeProvider *cameraSizeProvider)
		: m_cameraSizeProvider(cameraSizeProvider)
	{
	}
	TUnitConverter *clone() const { return new ZDepthUnitConverter(m_cameraSizeProvider); }
	inline double getCameraSize() const { return (*m_cameraSizeProvider)(); }
	double convertTo(double v) const
	{
		return (1 - v * 0.001) * getCameraSize();
	}
	double convertFrom(double v) const
	{
		return (1 - v / getCameraSize()) * 1000.0;
	}
};

//-------------------------------------------------------------------

class CameraZDepthUnitConverter : public TUnitConverter
{
	TMeasureManager::CameraSizeProvider *m_cameraSizeProvider;

public:
	CameraZDepthUnitConverter(TMeasureManager::CameraSizeProvider *cameraSizeProvider)
		: m_cameraSizeProvider(cameraSizeProvider)
	{
	}
	TUnitConverter *clone() const { return new CameraZDepthUnitConverter(m_cameraSizeProvider); }
	inline double getCameraSize() const { return (*m_cameraSizeProvider)(); }
	double convertTo(double v) const
	{
		return (1 + v * 0.001) * getCameraSize();
	}
	double convertFrom(double v) const
	{
		return (v / getCameraSize() - 1) * 1000.0;
	}
};

//===================================================================
/*-- Zのカーブのハンドルの長さは0=0となるようにしなければならない --*/
class ZDepthHandleUnitConverter : public TUnitConverter
{

	TMeasureManager::CameraSizeProvider *m_cameraSizeProvider;

public:
	ZDepthHandleUnitConverter(TMeasureManager::CameraSizeProvider *cameraSizeProvider)
		: m_cameraSizeProvider(cameraSizeProvider)
	{
	}

	TUnitConverter *clone() const { return new ZDepthHandleUnitConverter(m_cameraSizeProvider); }
	inline double getCameraSize() const { return (*m_cameraSizeProvider)(); }
	double convertTo(double v) const
	{
		return -v * 0.001 * getCameraSize();
	}
	double convertFrom(double v) const
	{
		return (-v / getCameraSize()) * 1000.0;
	}
};

class CameraZDepthHandleUnitConverter : public TUnitConverter
{

	TMeasureManager::CameraSizeProvider *m_cameraSizeProvider;

public:
	CameraZDepthHandleUnitConverter(TMeasureManager::CameraSizeProvider *cameraSizeProvider)
		: m_cameraSizeProvider(cameraSizeProvider)
	{
	}

	TUnitConverter *clone() const { return new CameraZDepthHandleUnitConverter(m_cameraSizeProvider); }
	inline double getCameraSize() const { return (*m_cameraSizeProvider)(); }
	double convertTo(double v) const
	{
		return v * 0.001 * getCameraSize();
	}
	double convertFrom(double v) const
	{
		return (v / getCameraSize()) * 1000.0;
	}
};

} // namespace

//-------------------------------------------------------------------

void TMeasureManager::addCameraMeasures(CameraSizeProvider *cameraSizeProvider)
{
	TUnit
		u0(L"z"),
		u1(L"fld", new ZDepthUnitConverter(cameraSizeProvider)),
		u2(L"fld", new CameraZDepthUnitConverter(cameraSizeProvider)),
		u3(L"fld", new ZDepthHandleUnitConverter(cameraSizeProvider)),
		u4(L"fld", new CameraZDepthHandleUnitConverter(cameraSizeProvider));

	TMeasure *zdepth = new TMeasure("zdepth", u0.clone());
	TUnit *u = u1.clone();
	zdepth->add(u);
	zdepth->setCurrentUnit(u);
	zdepth->setStandardUnit(u);
	TMeasureManager::instance()->add(zdepth);

	zdepth = new TMeasure("zdepth.cam", u0.clone());
	u = u2.clone();
	zdepth->add(u);
	zdepth->setCurrentUnit(u);
	zdepth->setStandardUnit(u);

	TMeasureManager::instance()->add(zdepth);

	zdepth = new TMeasure("zdepth.handle", u0.clone());
	u = u3.clone();
	zdepth->add(u);
	zdepth->setCurrentUnit(u);
	zdepth->setStandardUnit(u);
	TMeasureManager::instance()->add(zdepth);

	zdepth = new TMeasure("zdepth.cam.handle", u0.clone());
	u = u4.clone();
	zdepth->add(u);
	zdepth->setCurrentUnit(u);
	zdepth->setStandardUnit(u);
	TMeasureManager::instance()->add(zdepth);
}
