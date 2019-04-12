#pragma once
#include "Math.h"
#include "Structs.h"
#include "Config.h"
#include "Color.h"
#include "Engine.h"

#include <string>
#include <functional>
#include <array>

#define NO_CONDITION {[]() -> bool {return true;}}
#define M_C(CAPLIST, COND) {[CAPLIST]() -> bool {return COND;}}

constexpr LONG FONT_HEIGHT = 10;
constexpr LONG FONT_TITLE_HEIGHT = FONT_HEIGHT * g_Math.PHI;

constexpr LONG OBJECT_GAP = FONT_HEIGHT - 2;
constexpr LONG OBJECT_HEIGHT = FONT_HEIGHT;

constexpr LONG SHORT_WIDTH = 10;
constexpr LONG LONG_WIDTH = 80;

constexpr LONG WINDOW_COUNT = 6;
constexpr LONG MENU_WIDTH = LONG_WIDTH * g_Math.PHI;
constexpr LONG MENU_HEIGHT = FONT_HEIGHT + 4 + OBJECT_HEIGHT * (WINDOW_COUNT + 1) + OBJECT_GAP * (WINDOW_COUNT + 2); //qu1ck maffs

template <typename T>
struct CManagedMenuItem
{
	CManagedMenuItem() = delete;
	CManagedMenuItem(T* Item, bool bManaged);
	~CManagedMenuItem();

	T* operator()();
	const T* operator()() const;

	T* operator->();
	const T* operator->() const;

	T* m_Item;
	bool m_bManaged;
};

class CCondition
{
private:
	std::function<bool(void)> m_Condition;

public:
	CCondition(const std::function<bool(void)>& Condition = { []() {return true; } });
	~CCondition() = default;

	CCondition& operator=(const std::function<bool(void)>& New);

	bool PassesCondition() const;
	void SetNewTarget(const std::function<bool(void)>& New);
};

class CBaseRenderable
{
protected:
	RECT m_RenderBounds;
	char m_RenderText[BUFFER_SIZE];
	CCondition m_Condition;

public:
	CBaseRenderable() = delete;
	CBaseRenderable(const RECT& RenderBounds, const char* RenderText, const CCondition& Condition = NO_CONDITION);

	virtual ~CBaseRenderable() = default;

	virtual void Print() = 0;
	virtual bool IsVisible() const;

	const char* GetRenderText() const;
	const RECT& GetRenderBounds() const;

	void SetRenderBounds(const RECT& New)
	{
		m_RenderBounds = New;
	}

	CCondition& GetCondition();
	void SetCondition(const CCondition& New);
};

class CBaseControlable : public CBaseRenderable
{
public:
	CBaseControlable() = delete;
	CBaseControlable(const RECT & RenderBounds, const char* RenderText, const CCondition& Condition = NO_CONDITION);

	virtual ~CBaseControlable() = default;
	virtual void Control() = 0;
};

class CBaseChildSaveable
{
public:
	CBaseChildSaveable() = default;

	virtual void Save(CConfig& out) = 0;
	virtual void Load(CConfig& in) = 0;
};

enum class ConfigFile_t
{
	level_0,
	level_1,
	level_2,
	level_3,
	level_4,
	level_5
};

class CBaseParentSaveable
{
public:
	CBaseParentSaveable() = default;

	virtual void Save(ConfigFile_t Reason) = 0;
	virtual void Load(ConfigFile_t Reason) = 0;
};

class CBaseElement : public CBaseControlable, public CBaseChildSaveable
{
protected:
	POINT m_CurrentRenderPosition;

public:
	CBaseElement(const char* RenderText, const CCondition& Condition = NO_CONDITION);
	~CBaseElement() = default;

	virtual size_t GetVerticalAdjustment() const = 0;
	virtual void SetRenderPosition(const POINT& New);
};

class CCheckbox : public CBaseElement
{
private:
	bool m_Value;
	size_t m_Width, m_Height, m_Gap;

public:
	CCheckbox(int w, int h, int gap, const char* Text, const CCondition& Condition = NO_CONDITION);
	~CCheckbox() = default;

	bool Value() const;
	void SetValue(bool New);

	void Print() override;
	void Control() override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;

	size_t GetVerticalAdjustment() const override;
};

class CCycle : public CBaseElement
{
private:
	size_t m_Value;
	size_t m_Width, m_Height, m_Gap;
	std::vector<const char*> m_Items;

	void Clamp();

public:
	CCycle(int w, int h, int gap, const char* Text, const std::vector<const char*>& Items, const CCondition& Condition = NO_CONDITION);
	~CCycle() = default;

	size_t Value() const;
	void SetValue(size_t New);

	void Print() override;
	void Control() override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;

	size_t GetVerticalAdjustment() const override;
};

class CTextNote : public CBaseElement
{
public:
	CTextNote(const char* Text, const CCondition& Condition = NO_CONDITION);
	~CTextNote() = default;

	void Print() override;
	void Control() override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;

	size_t GetVerticalAdjustment() const override;
};

class CButton : public CBaseElement
{
private:
	std::function<void(void)> m_CallBack;
	size_t m_Width, m_Height;

public:
	CButton(int w, int h, const char* Text, const std::function<void(void)>& Callback, const CCondition& Condition = NO_CONDITION);
	~CButton() = default;

	const std::function<void(void)>& CallBack() const;

	void Print() override;
	void Control() override;

	size_t GetVerticalAdjustment() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;
};

class CKeySelect : public CBaseElement
{
private:
	key_t m_Value;
	CCheckbox* m_pShouldToggle;
	size_t m_Width, m_Height, m_Gap;
	bool m_bWaitingOnValue;

public:
	CKeySelect(int w, int h, int gap, const char* Text, const CCondition& Condition = NO_CONDITION);
	~CKeySelect() = default;

	key_t Value() const;
	void SetValue(key_t New);

	bool Toggle() const;
	void SetToggle(bool New);

	void Print() override;
	void Control() override;

	size_t GetVerticalAdjustment() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;
};

template <typename T>
struct SpecialValue_t
{
	T val;
	const char* output;
};

template <typename T>
class CNumericalInput : public CBaseElement
{
private:
	const SpecialValue_t<T> m_Low;
	const SpecialValue_t<T> m_High;
	size_t m_Width, m_Height, m_Gap;
	bool m_bWaitingOnValue;
	std::string buf;
	T m_Value;

public:
	CNumericalInput(int w, int h, int gap, const char* Text, const SpecialValue_t<T>& High, const SpecialValue_t<T>& Low, const CCondition& Condition = NO_CONDITION);
	~CNumericalInput() = default;

	const T& Value() const;
	void SetValue(const T& New);

	void Print() override;
	void Control() override;

	size_t GetVerticalAdjustment() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;
};

template <typename T>
class CSlider : public CBaseElement
{
private:
	T m_Value;
	const SpecialValue_t<T> m_High, m_Low;
	size_t m_Width, m_Height, m_Gap;
	char buf[BUFFER_SIZE];

public:
	CSlider(int w, int h, int gap, const char* Text, const SpecialValue_t<T>& High, const SpecialValue_t<T>& Low, const CCondition& Condition = NO_CONDITION);
	~CSlider() = default;

	const T& Value() const;
	void SetValue(const T& New);

	void Print() override;
	void Control() override;

	size_t GetVerticalAdjustment() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;
};

class CColorSelect : public CBaseElement
{
private:
	size_t m_Width, m_Height, m_Gap;
	char buf[BUFFER_SIZE];
	CSlider<unsigned char>* m_pRSlider;
	CSlider<unsigned char>* m_pGSlider;
	CSlider<unsigned char>* m_pBSlider;
	bool m_bOpen;

public:
	CColorSelect(int w, int h, int gap, const char* Text, const CCondition& Condition = NO_CONDITION);
	~CColorSelect();

	CColor Value() const;
	void SetValue(const CColor& New);

	void Print() override;
	void Control() override;

	size_t GetVerticalAdjustment() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;
};

class CUniSelect : public CBaseElement
{
private:
	std::vector<const char*> m_Items;
	size_t m_Value;
	size_t m_Width, m_HeightSingle, m_Gap;
	bool m_bIsOpen;
	char buf[BUFFER_SIZE];

public:
	CUniSelect(int w, int hsingle, int gap, const char* Text, const std::vector<const char*>& Items, const CCondition& Condition = NO_CONDITION);
	~CUniSelect() = default;

	size_t Value() const;
	void SetValue(size_t New);

	id_t AddItem(const char* text);

	void Print() override;
	void Control() override;

	size_t GetVerticalAdjustment() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;
};

class CMultiSelect : public CBaseElement
{
private:
	std::vector<const char*> m_Items;
	std::vector<size_t> m_SelectedItems;
	size_t m_Width, m_HeightSingle, m_Gap;
	bool m_bIsOpen;
	char buf[BUFFER_SIZE];

public:
	CMultiSelect(int w, int hsingle, int gap, const char* Text, const std::vector<const char*>& Items, const CCondition& Condition = NO_CONDITION);
	~CMultiSelect() = default;

	const std::vector<size_t>& Value() const;
	id_t AddItem(const char* text);

	void Print() override;
	void Control() override;

	size_t GetVerticalAdjustment() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;
};

class CMultiSelectToggleList : public CBaseElement
{
public:
	struct Item_t
	{
	public:
		Item_t(const char* szText, bool bSelected) :
			m_szText(szText), m_bSelected(bSelected)
		{
			;
		}

		const char* m_szText;
		bool m_bSelected;
	};

private:
	std::vector<Item_t> m_Items;
	size_t m_Width, m_HeightSingle, m_Gap;
	bool m_bIsOpen;
	const bool m_bForceOpen;
	char buf[BUFFER_SIZE];

public:
	CMultiSelectToggleList(int w, int hsingle, int gap, const char* Text, const std::vector<const char*>& Items, bool ForceOpen = false, const CCondition& Condition = NO_CONDITION);
	~CMultiSelectToggleList() = default;

	const std::vector<Item_t>& Value() const;
	id_t AddItem(const char* text);

	void Print() override;
	void Control() override;

	size_t GetVerticalAdjustment() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;
};

class CSubSection : public CBaseRenderable, public CBaseChildSaveable
{
private:
	std::vector<CManagedMenuItem<CBaseElement>> m_Elements;

public:
	CSubSection(const char* Text, const CCondition& Condition = NO_CONDITION);
	~CSubSection() = default;

	id_t AddElement(CBaseElement* E, bool bTransferOwnership = true);

	id_t AddNewCheckBox(const char* szText, const CCondition& Condition = NO_CONDITION);
	id_t AddNewButton(const char* szText, const std::function<void(void)>& CallBack, const CCondition& Condition = NO_CONDITION);

	template <typename T>
	id_t AddNewSlider(const char* szText, const SpecialValue_t<T>& High, const SpecialValue_t<T>& Low, const CCondition& Condition = NO_CONDITION)
	{
		m_Elements.emplace_back(new CSlider<T>(LONG_WIDTH, OBJECT_HEIGHT, OBJECT_GAP, szText, High, Low, Condition), true);
		return m_Elements.size() - 1;
	}

	id_t AddNewKeySelect(const char* szText, const CCondition& Condition = NO_CONDITION);
	id_t AddNewColorSelect(const char* szText, const CCondition& Condition = NO_CONDITION);
	id_t AddNewUniSelect(const char* szText, const std::vector<const char*>& Items, const CCondition& Condition = NO_CONDITION);
	id_t AddNewMultiSelect(const char* szText, const std::vector<const char*>& Items, const CCondition& Condition = NO_CONDITION);
	id_t AddNewMultiSelectToggleList(const char* szText, const std::vector<const char*>& Items, bool ForceOpen = false, const CCondition& Condition = NO_CONDITION);
	id_t AddNewTextNote(const char* szText, const CCondition& Condition = NO_CONDITION);

	void Print() override;
	void Control();

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;

	void Clear();

	const std::vector<CManagedMenuItem<CBaseElement>>& GetElements() const;
	const CBaseElement& GetElement(id_t nID) const;

	std::vector<CManagedMenuItem<CBaseElement>>& GetElements();
	CBaseElement& GetElement(id_t nID);
};

class CTab : public CBaseControlable, public CBaseChildSaveable
{
private:
	std::vector<CManagedMenuItem<CSubSection>> m_SubSections;

	size_t GetVisibleSubSectionCount() const;

public:
	CTab(const char* Text, const CCondition& Condition = NO_CONDITION);
	~CTab() = default;

	id_t AddSubSection(CSubSection* S, bool bTransferOwnership = true);
	id_t AddNewSubSection(const char* szText, const CCondition& Condition = NO_CONDITION);

	void Print() override;
	void Control() override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;

	const std::vector<CManagedMenuItem<CSubSection>>& GetSubSections() const;
	const CSubSection& GetSubSection(id_t nID) const;

	std::vector<CManagedMenuItem<CSubSection>>& GetSubSections();
	CSubSection& GetSubSection(id_t nID);
};

class CWindow : public CBaseControlable, public CBaseChildSaveable
{
private:
	POINT m_LastCursor;
	POINT m_LastCursor_Resize;

	std::vector<CManagedMenuItem<CTab>> m_Tabs;
	size_t m_CurrentTab;
	bool m_bVisible, m_bIsDragging, m_bIsResizing;

	bool SetCurrentTab();
	size_t GetVisibleTabCount() const;

	bool DragWindow();
	bool ResizeWindow();

public:
	CWindow(const RECT& Bounds, const char* RenderText, const CCondition& Condition = NO_CONDITION);
	~CWindow() = default;

	id_t AddTab(CTab* T, bool bTransferOwnerShip = true);
	id_t AddNewTab(const char* szText, const CCondition& Condition = NO_CONDITION);

	void Print() override;
	void Control() override;

	bool IsVisible() const override;

	void Save(CConfig& cfg) override;
	void Load(CConfig& cfg) override;

	void OnFailPrint();

	void SetVisible(bool New);
	bool GetVisible() const;

	const std::vector<CManagedMenuItem<CTab>>& GetTabs() const;
	const CTab& GetTab(id_t nID) const;

	std::vector<CManagedMenuItem<CTab>>& GetTabs();
	CTab& GetTab(id_t nID);
};

struct DebugMessage_t
{
private:
	char* m_szMessage;
	CTimer m_Time;
	float m_flMaxTime;

public:
	DebugMessage_t() = delete;
	DebugMessage_t(const char* const szMessage, const float flMaxTime) : 
		m_szMessage(new char[BUFFER_SIZE]), m_Time(), m_flMaxTime(flMaxTime)
	{
		memset(m_szMessage, 0, BUFFER_SIZE);
		memcpy(m_szMessage, szMessage, min(strlen(szMessage), BUFFER_SIZE));
	}

	DebugMessage_t(const DebugMessage_t& rhs) :
		m_szMessage(new char[BUFFER_SIZE]), m_Time(rhs.m_Time), m_flMaxTime(rhs.m_flMaxTime)
	{
		memset(m_szMessage, 0, BUFFER_SIZE);
		memcpy(m_szMessage, rhs.m_szMessage, min(strlen(rhs.m_szMessage), BUFFER_SIZE));
	}

	~DebugMessage_t()
	{
		delete m_szMessage;
	}

	DebugMessage_t& operator=(const DebugMessage_t& rhs)
	{
		memset(m_szMessage, 0, BUFFER_SIZE);
		memcpy(m_szMessage, rhs.m_szMessage, min(strlen(rhs.m_szMessage), BUFFER_SIZE));

		m_Time = rhs.m_Time;
		m_flMaxTime = rhs.m_flMaxTime;

		return *this;
	}

	bool IsValid() const
	{
		return m_Time.Elapsed() < m_flMaxTime;
	}

	const char* const szGetMessage() const
	{
		return m_szMessage;
	}
};

class IMenu : public CBaseControlable, public CBaseParentSaveable
{
private:
	POINT m_LastCursor;
	POINT m_LastCursor_Resize;

	std::vector<CManagedMenuItem<CWindow>> m_Windows;
	std::vector<CManagedMenuItem<CButton>> m_Buttons;
	std::vector<DebugMessage_t> m_szDebugMessages;

	bool m_bVisible, m_bIsDragging, m_bIsResizing;

	bool m_bMouse1Pressed, m_bMouse1Down;
	POINT m_CursorPosition;

	bool DragMenu();
	bool ResizeMenu();

public:
	IMenu(const RECT& Bounds, const char* RenderText, const CCondition& Condition = NO_CONDITION);
	~IMenu() = default;

	id_t AddWindow(CWindow* W, bool bTransferOwnerShip = true);
	id_t AddNewWindow(const RECT& Bounds, const char* RenderText, const CCondition& Condition = NO_CONDITION);

	id_t AddButton(CButton* B, bool bTransferOwnerShip = true);
	id_t AddNewButton(int w, int h, const char* Text, const std::function<void(void)>& Callback, const CCondition& Condition = NO_CONDITION);

	__forceinline id_t AddNewDebugMessage(const char* const szMessage, const float flMaxTime)
	{
		m_szDebugMessages.emplace_back(szMessage, flMaxTime);

		return m_szDebugMessages.size() - 1;
	}

	void Print() override;
	void Control() override;

	bool IsVisible() const override;

	void Save(ConfigFile_t Reason) override;
	void Load(ConfigFile_t Reason) override;

	bool Mouse1Pressed() const;
	bool Mouse1Down() const;

	bool CursorInBounds(const RECT& Bounds) const;

	__forceinline const POINT& GetCursorPosition()
	{
		return m_CursorPosition;
	}

	const std::vector<CManagedMenuItem<CWindow>>& GetWindows() const;
	const CWindow& GetWindow(id_t nID) const;

	std::vector<CManagedMenuItem<CWindow>>& GetWindows();
	CWindow& GetWindow(id_t nID);


	const std::vector<CManagedMenuItem<CButton>>& GetButtons() const;
	const CButton& GetButton(id_t nID) const;

	std::vector<CManagedMenuItem<CButton>>& GetButtons();
	CButton& GetButton(id_t nID);

	static CColor GetOutlineColor();
	static const HBRUSH& GetOutlineBrush();
	static CColor GetSelectColor();
	static const HBRUSH& GetSelectBrush();
	static CColor GetBackGroundColor();
	static const HBRUSH& GetBackGroundBrush();
	static CColor GetTextColor();
	static const HBRUSH& GetTextBrush();

	static const std::array<const char*, 6>& GetConfigNames();
	static const std::array<const char*, 124>& GetKeyNames();
	static constexpr int GetKeyCount();
};

extern IMenu g_Menu;

template<typename T>
inline CNumericalInput<T>::CNumericalInput(int w, int h, int gap, const char * Text, const SpecialValue_t<T>& High, const SpecialValue_t<T>& Low, const CCondition & Condition) :
	CBaseElement(Text, Condition), m_Width(w), m_Height(h), m_Gap(gap), m_Value(T(0)), m_High(High), m_Low(Low), m_bWaitingOnValue(false)
{
	m_Value = std::clamp(m_Value, m_Low.val, m_High.val);
}

template<typename T>
inline const T & CNumericalInput<T>::Value() const
{
	return m_Value;
}

template<typename T>
inline void CNumericalInput<T>::SetValue(const T & New)
{
	m_Value = New;
}

template<typename T>
inline void CNumericalInput<T>::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_Height };

	g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_Height, 1, IMenu::GetOutlineBrush());

	if (!m_bWaitingOnValue)
	{
		buf = std::to_string(m_Value);
	}
	else
	{
		const auto w = g_Render.GetTextWidth(buf.c_str());
		g_Render.Line((m_RenderBounds.left + m_RenderBounds.right + w) / 2 + 1, m_RenderBounds.top + 1,
			(m_RenderBounds.left + m_RenderBounds.right + w) / 2 + 1, m_RenderBounds.bottom - 1, 1, IMenu::GetTextColor());
	}

	g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), buf.c_str());
	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_RenderBounds.top, IMenu::GetTextColor(), m_RenderText);
}

template<typename T>
inline void CNumericalInput<T>::Control()
{
	if (g_Menu.Mouse1Pressed() && g_Menu.CursorInBounds(m_RenderBounds))
	{
		m_bWaitingOnValue = !m_bWaitingOnValue;

		if (m_bWaitingOnValue)
		{
			buf.clear();
		}
		else
		{
			if (buf.size() > 0 && buf.size() <= 10)
			{
				if constexpr (std::is_floating_point<T>::value)
				{
					m_Value = std::stof(buf.c_str());
				}
				else
				{
					m_Value = std::stoi(buf.c_str());
				}
			}

			m_Value = std::clamp(m_Value, m_Low.val, m_High.val);
		}
	}

	if (m_bWaitingOnValue)
	{
		for (int i = 48; i <= 57; i++)
		{
			if (g_Input.KeyPressed(i))
			{
				buf.append(std::to_string(i - 48));
			}
		}

		if constexpr (!std::is_unsigned<T>::value)
		{
			if (g_Input.KeyPressed(VK_OEM_MINUS) && buf.size() == 0)
			{
				buf.push_back('-');
			}
		}

		if constexpr (std::is_floating_point<T>::value)
		{
			if (g_Input.KeyPressed(VK_OEM_COMMA))
			{
				buf.append('.');
			}
		}

		if (g_Input.KeyPressed(VK_BACK) && buf.size())
		{
			buf.pop_back();
		}
	}
}

template<typename T>
inline size_t CNumericalInput<T>::GetVerticalAdjustment() const
{
	return m_Height;
}

template<typename T>
inline void CNumericalInput<T>::Save(CConfig & cfg)
{
	cfg.AddVar<T>(m_RenderText, m_Value);
}

template<typename T>
inline void CNumericalInput<T>::Load(CConfig & cfg)
{
	const auto var = cfg.GV<T>(m_RenderText);
	if (var.m_ID != -1)
	{
		m_Value = var.m_Value;
	}
	else
	{
		m_Value = m_Low.val;
	}
}

template<typename T>
inline CSlider<T>::CSlider(int w, int h, int gap, const char * Text, const SpecialValue_t<T>& High, const SpecialValue_t<T>& Low, const CCondition & Condition) :
	CBaseElement(Text, Condition), m_Width(w), m_Height(h), m_Gap(gap), m_Value(T(0)), m_High(High), m_Low(Low)
{
	memset(buf, 0, BUFFER_SIZE);
	m_Value = std::clamp(m_Value, m_Low.val, m_High.val);
}

template<typename T>
inline const T & CSlider<T>::Value() const
{
	return m_Value;
}

template<typename T>
inline void CSlider<T>::SetValue(const T & New)
{
	m_Value = New;
}

template<typename T>
inline void CSlider<T>::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
	m_CurrentRenderPosition.x + LONG(m_Width * ((float)m_Value / float(m_High.val - m_Low.val))), m_CurrentRenderPosition.y + (LONG)m_Height };

	g_Render.FilledBox({ m_RenderBounds.left, m_RenderBounds.top + 1, m_RenderBounds.right, m_RenderBounds.bottom }, IMenu::GetSelectBrush());
	m_RenderBounds.right = m_CurrentRenderPosition.x + m_Width;
	g_Render.BorderedBox(m_CurrentRenderPosition.x, m_CurrentRenderPosition.y, m_Width, m_Height, 1, IMenu::GetOutlineBrush());

	std::string txt = m_RenderText;
	txt.append(": ");

	if (strcmp(m_High.output, "") && m_Value == m_High.val)
	{
		txt.append(m_High.output);
	}
	else if (strcmp(m_Low.output, "") && m_Value == m_Low.val)
	{
		txt.append(m_Low.output);
	}
	else
	{
		if constexpr (std::is_floating_point<T>::value)
		{
			sprintf(buf, "%0.2f", m_Value);
		}
		else
		{
			sprintf(buf, "%d", m_Value);
		}
		txt.append(buf);
	}

	CColor col = IMenu::GetTextColor();
	if (m_Value == m_Low.val && strcmp(m_Low.output, ""))
	{
		col.r() = std::clamp((int)col.r() - 50, 0, 255);
		col.g() = std::clamp((int)col.g() - 50, 0, 255);
		col.b() = std::clamp((int)col.b() - 50, 0, 255);
	}
	else if (m_Value == m_High.val && strcmp(m_High.output, ""))
	{
		col.r() = std::clamp((int)col.r() + 50, 0, 255);
		col.g() = std::clamp((int)col.g() + 50, 0, 255);
		col.b() = std::clamp((int)col.b() + 50, 0, 255);
	}

	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_RenderBounds.top, col, txt.c_str());
}

template<typename T>
inline void CSlider<T>::Control()
{
	const RECT Bounds = { m_RenderBounds.left, m_RenderBounds.top + 1, m_RenderBounds.right, m_RenderBounds.bottom };
	if (g_Menu.CursorInBounds(Bounds) && g_Menu.Mouse1Down())
	{
		m_Value = m_Low.val + (g_Menu.GetCursorPosition().x - Bounds.left) * (float(m_High.val - m_Low.val) / float(Bounds.right - Bounds.left));
	}
}

template<typename T>
inline size_t CSlider<T>::GetVerticalAdjustment() const
{
	return m_Height;
}

template<typename T>
inline void CSlider<T>::Save(CConfig & cfg)
{
	cfg.AddVar<T>(m_RenderText, m_Value);
}

template<typename T>
inline void CSlider<T>::Load(CConfig & cfg)
{
	const auto var = cfg.GV<T>(m_RenderText);
	if (var.m_ID != -1)
	{
		m_Value = var.m_Value;
	}
	else
	{
		m_Value = m_Low.val;
	}
}