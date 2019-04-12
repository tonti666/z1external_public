#include "Menu.h"
#include "Controls.h"

#include <algorithm>

IMenu g_Menu({MENU_WIDTH, MENU_HEIGHT, MENU_WIDTH * 2, MENU_HEIGHT * 2}, "z1");

CBaseRenderable::CBaseRenderable(const RECT & RenderBounds, const char * RenderText, const CCondition & Condition) :
	m_RenderBounds(RenderBounds), m_Condition(Condition)
{
	memset(m_RenderText, 0, BUFFER_SIZE);
	memcpy(m_RenderText, RenderText, strlen(RenderText));
}

bool CBaseRenderable::IsVisible() const
{
	return m_Condition.PassesCondition();
}

const char* CBaseRenderable::GetRenderText() const
{
	return m_RenderText;
}

const RECT & CBaseRenderable::GetRenderBounds() const
{
	return m_RenderBounds;
}

CCondition & CBaseRenderable::GetCondition()
{
	return m_Condition;
}

void CBaseRenderable::SetCondition(const CCondition & New)
{
	m_Condition = New;
}

CBaseControlable::CBaseControlable(const RECT & RenderBounds, const char* RenderText, const CCondition& Condition)
	: CBaseRenderable(RenderBounds, RenderText, Condition)
{
}

bool CWindow::SetCurrentTab()
{
	if (!g_Menu.Mouse1Pressed())
	{
		return false;
	}

	RECT Current = m_RenderBounds;
	Current.top += 3 + FONT_HEIGHT;
	Current.bottom = Current.top + FONT_HEIGHT + 1;
	if (!(g_Menu.GetCursorPosition().y >= Current.top && g_Menu.GetCursorPosition().y <= Current.bottom))
	{
		return false;
	}

	const size_t HorizontalGap = (Current.right - Current.left) / (GetVisibleTabCount());
	Current.right = Current.left + HorizontalGap;
	Current.left -= 1; Current.right -= 1;
	for (size_t i = 0; i < m_Tabs.size(); i++)
	{
		if (!m_Tabs[i]->IsVisible())
		{
			continue;
		}

		if (g_Menu.GetCursorPosition().x >= Current.left && g_Menu.GetCursorPosition().x <= Current.right)
		{
			m_CurrentTab = i;
			return true;
		}

		Current.left += HorizontalGap;
		Current.right += HorizontalGap;
	}

	return false;
}

size_t CWindow::GetVisibleTabCount() const
{
	size_t inc = 0;
	for (size_t it = 0; it < m_Tabs.size(); it++)
	{
		if (m_Tabs[it]->IsVisible())
		{
			inc++;
		}
	}

	return inc;
}

bool CWindow::DragWindow()
{
	if (!m_bIsDragging)
	{
		m_LastCursor = g_Menu.GetCursorPosition();
	}

	const RECT Bounds = { m_RenderBounds.left - TERANY(m_bIsDragging, 100, 0), m_RenderBounds.top - TERANY(m_bIsDragging, 100, 0), 
		m_RenderBounds.right + TERANY(m_bIsDragging, 100, 0), m_RenderBounds.top + FONT_HEIGHT + 1 + TERANY(m_bIsDragging, 100, 0) };
	m_bIsDragging = g_Menu.CursorInBounds(Bounds) && g_Menu.Mouse1Down();
	
	if (m_bIsDragging)
	{
		const POINT Delta = { g_Menu.GetCursorPosition().x - m_LastCursor.x, g_Menu.GetCursorPosition().y - m_LastCursor.y };
		m_RenderBounds.left += Delta.x; m_RenderBounds.right += Delta.x;
		m_RenderBounds.bottom += Delta.y; m_RenderBounds.top += Delta.y;
	}

	m_LastCursor = g_Menu.GetCursorPosition();
	return m_bIsDragging;
}

bool CWindow::ResizeWindow()
{
	if (!m_bIsResizing)
	{
		m_LastCursor_Resize = g_Menu.GetCursorPosition();
	}

	const RECT Bounds = { m_RenderBounds.right - 1 - TERANY(m_bIsResizing, 100, SHORT_WIDTH), m_RenderBounds.bottom - 1 - TERANY(m_bIsResizing, 100, OBJECT_HEIGHT),
		m_RenderBounds.right + TERANY(m_bIsResizing, 100, 0), m_RenderBounds.bottom + TERANY(m_bIsResizing, 100, 0) };
	m_bIsResizing = g_Menu.CursorInBounds(Bounds) && g_Menu.Mouse1Down();

	if (m_bIsResizing)
	{
		const POINT Delta = { g_Menu.GetCursorPosition().x - m_LastCursor_Resize.x, g_Menu.GetCursorPosition().y - m_LastCursor_Resize.y };
		m_RenderBounds.right += Delta.x;
		m_RenderBounds.bottom += Delta.y; 
	}

	m_LastCursor_Resize = g_Menu.GetCursorPosition();
	return m_bIsResizing;
}

CWindow::CWindow(const RECT & Bounds, const char * RenderText, const CCondition & Condition)
	: CBaseControlable(Bounds, RenderText, Condition), m_bVisible(false), m_bIsDragging(false), m_CurrentTab(0)
{
}

id_t CWindow::AddTab(CTab * T, bool bTransferOwnerShip)
{
	m_Tabs.emplace_back(T, bTransferOwnerShip);
	return m_Tabs.size() - 1;
}

id_t CWindow::AddNewTab(const char * szText, const CCondition & Condition)
{
	m_Tabs.emplace_back(new CTab(szText, Condition), true);
	return m_Tabs.size() - 1;
}

void CWindow::Print()
{
	if (!IsVisible())
	{
		return;
	}

	RECT Current = m_RenderBounds;
	g_Render.BorderedBox(Current.left - 1, Current.top - 1, Current.right - Current.left + 1, Current.bottom - Current.top + 1, 1, IMenu::GetTextBrush());
	g_Render.FilledBox(Current, IMenu::GetBackGroundBrush());
	Current.top += 1;
	g_Render.StringCentred((Current.left + Current.right) / 2, Current.top, IMenu::GetTextColor(), m_RenderText);
	Current.top += FONT_HEIGHT + 1;
	g_Render.Line(Current.left, Current.top, Current.right, Current.top, 1, IMenu::GetTextColor());
	Current.top += 1;
	g_Render.BorderedBox(Current.right - 1 - SHORT_WIDTH, Current.bottom - 1 - OBJECT_HEIGHT, SHORT_WIDTH, OBJECT_HEIGHT, 1, TERANY(m_bIsResizing, IMenu::GetSelectBrush(), IMenu::GetOutlineBrush()));

	const auto VisibleCount = GetVisibleTabCount();
	if (!VisibleCount)
	{
		return;
	}

	const LONG HorizontalGap = (Current.right - Current.left) / VisibleCount;
	for (size_t i = 0; i < m_Tabs.size(); i++)
	{
		if (!m_Tabs[i]->IsVisible())
		{
			continue;
		}

		g_Render.StringCentred((Current.left + HorizontalGap * i + Current.left + HorizontalGap * (i + 1)) / 2, Current.top, IMenu::GetTextColor(), m_Tabs[i]->GetRenderText());
		g_Render.Line(Current.left + HorizontalGap * i - 1, Current.top, Current.left + HorizontalGap * i - 1, Current.top + FONT_HEIGHT + 1, 1, IMenu::GetTextColor());
	}

	Current.top += FONT_HEIGHT + 1;
	g_Render.Line(Current.left, Current.top, Current.right, Current.top, 1, IMenu::GetTextColor());
	Current.top += 1;

	Current.left = m_RenderBounds.left;
	Current.right = m_RenderBounds.right;
	m_Tabs[m_CurrentTab]->SetRenderBounds(Current);
	m_Tabs[m_CurrentTab]->Print();
}

void CWindow::Control()
{
	if (!IsVisible())
	{
		m_bIsDragging = false;
		m_bIsResizing = false;
		return;
	}

	if (DragWindow())
	{
		m_bIsResizing = false;
		return;
	}

	if (ResizeWindow())
	{
		return;
	}

	if (!g_Menu.CursorInBounds(m_RenderBounds))
	{
		return;
	}

	if (SetCurrentTab())
	{
		return;
	}

	m_Tabs[m_CurrentTab]->Control();
}

bool CWindow::IsVisible() const
{
	return m_bVisible && m_Condition.PassesCondition();
}

void CWindow::Save(CConfig & cfg)
{
	for (auto& i : m_Tabs)
	{
		i->Save(cfg);
	}
}

void CWindow::Load(CConfig & cfg)
{
	for (auto& i : m_Tabs)
	{
		i->Load(cfg);
	}
}

void CWindow::OnFailPrint()
{
	m_bIsDragging = false;
}

void CWindow::SetVisible(bool New)
{
	m_bVisible = New;
}

bool CWindow::GetVisible() const
{
	return m_bVisible;
}

const std::vector<CManagedMenuItem<CTab>>& CWindow::GetTabs() const
{
	return m_Tabs;
}

const CTab & CWindow::GetTab(id_t nID) const
{
	return *(m_Tabs[nID]());
}

std::vector<CManagedMenuItem<CTab>>& CWindow::GetTabs()
{
	return m_Tabs;
}

CTab & CWindow::GetTab(id_t nID)
{
	return *(m_Tabs[nID]());
}
size_t CTab::GetVisibleSubSectionCount() const
{
	size_t inc = 0;
	for (size_t it = 0; it < m_SubSections.size(); it++)
	{
		if (m_SubSections[it]->IsVisible())
		{
			inc++;
		}
	}

	return inc;
}

CTab::CTab(const char * Text, const CCondition & Condition)
	: CBaseControlable({}, Text, Condition)
{
}

id_t CTab::AddSubSection(CSubSection * S, bool bTransferOwnership)
{
	m_SubSections.emplace_back(S, bTransferOwnership);
	return m_SubSections.size() - 1;
}

id_t CTab::AddNewSubSection(const char * szText, const CCondition & Condition)
{
	m_SubSections.emplace_back(new CSubSection(szText, Condition), true);
	return m_SubSections.size() - 1;
}

void CTab::Print()
{
	auto Current = m_RenderBounds;
	const auto VisibleCount = GetVisibleSubSectionCount();
	if (!VisibleCount)
	{
		return;
	}

	const LONG HorizontalGap = (Current.right - Current.left) / VisibleCount;
	Current.right = Current.left + HorizontalGap;
	for (size_t i = 0; i < m_SubSections.size(); i++)
	{
		if (!m_SubSections[i]->IsVisible())
		{
			continue;
		}

		g_Render.StringCentred((Current.left + Current.right) / 2, Current.top, IMenu::GetTextColor(), m_SubSections[i]->GetRenderText());
		g_Render.Line(Current.left - 1, Current.top, Current.left - 1, Current.bottom, 1, IMenu::GetTextColor());

		Current.top += (FONT_HEIGHT + 1);
		m_SubSections[i]->SetRenderBounds(Current);
		m_SubSections[i]->Print();
		Current.top -= (FONT_HEIGHT + 1);

		Current.left += HorizontalGap; Current.right += HorizontalGap;
	}

	Current.top += FONT_HEIGHT + 1;
	g_Render.Line(m_RenderBounds.left, Current.top, m_RenderBounds.right, Current.top, 1, IMenu::GetTextColor());
}

void CTab::Control()
{
	const auto GetSelectedTabID = [this]() -> id_t
	{
		auto Current = m_RenderBounds;
		const LONG HorizontalGap = (Current.right - Current.left) / (GetVisibleSubSectionCount());

		Current.right = Current.left + HorizontalGap;
		Current.left -= 1; Current.right -= 1;
		for (size_t i = 0; i < m_SubSections.size(); i++)
		{
			if (!m_SubSections[i]->IsVisible())
			{
				continue;
			}

			if (g_Menu.CursorInBounds(Current))
			{
				return i;
			}

			Current.left += HorizontalGap; Current.right += HorizontalGap;
		}

		return -1;
	};

	const auto SelectedTab = GetSelectedTabID();
	if (SelectedTab == -1)
	{
		return;
	}

	m_SubSections[SelectedTab]->Control();
}

void CTab::Save(CConfig & cfg)
{
	for (auto& i : m_SubSections)
	{
		i->Save(cfg);
	}
}

void CTab::Load(CConfig & cfg)
{
	for (auto& i : m_SubSections)
	{
		i->Load(cfg);
	}
}

const std::vector<CManagedMenuItem<CSubSection>>& CTab::GetSubSections() const
{
	return m_SubSections;
}

const CSubSection & CTab::GetSubSection(id_t nID) const
{
	return *(m_SubSections[nID]());
}

std::vector<CManagedMenuItem<CSubSection>>& CTab::GetSubSections() 
{
	return m_SubSections;
}

CSubSection & CTab::GetSubSection(id_t nID) 
{
	return *(m_SubSections[nID]());
}

CSubSection::CSubSection(const char * Text, const CCondition & Condition)
	: CBaseRenderable({}, Text, Condition)
{
}

id_t CSubSection::AddElement(CBaseElement * E, bool bTransferOwnership)
{
	m_Elements.emplace_back(E, bTransferOwnership);
	return m_Elements.size() - 1;
}

id_t CSubSection::AddNewCheckBox(const char* szText, const CCondition& Condition)
{
	m_Elements.emplace_back(new CCheckbox(SHORT_WIDTH, OBJECT_HEIGHT, OBJECT_GAP, szText, Condition), true);
	return m_Elements.size() - 1;
}

id_t CSubSection::AddNewButton(const char* szText, const std::function<void(void)>& CallBack, const CCondition& Condition)
{
	m_Elements.emplace_back(new CButton(LONG_WIDTH, OBJECT_HEIGHT, szText, CallBack, Condition), true);
	return m_Elements.size() - 1;
}

id_t CSubSection::AddNewKeySelect(const char* szText, const CCondition& Condition)
{
	m_Elements.emplace_back(new CKeySelect(LONG_WIDTH, OBJECT_GAP, OBJECT_GAP, szText, Condition), true);
	return m_Elements.size() - 1;
}

id_t CSubSection::AddNewColorSelect(const char* szText, const CCondition& Condition)
{
	m_Elements.emplace_back(new CColorSelect(LONG_WIDTH, OBJECT_HEIGHT, OBJECT_GAP, szText, Condition), true);
	return m_Elements.size() - 1;
}

id_t CSubSection::AddNewUniSelect(const char* szText, const std::vector<const char*>& Items, const CCondition& Condition)
{
	m_Elements.emplace_back(new CUniSelect(LONG_WIDTH, OBJECT_HEIGHT, OBJECT_GAP, szText, Items, Condition), true);
	return m_Elements.size() - 1;
}

id_t CSubSection::AddNewMultiSelect(const char* szText, const std::vector<const char*>& Items, const CCondition& Condition)
{
	m_Elements.emplace_back(new CMultiSelect(LONG_WIDTH, OBJECT_HEIGHT, OBJECT_GAP, szText, Items, Condition), true);
	return m_Elements.size() - 1;
}

id_t CSubSection::AddNewMultiSelectToggleList(const char* szText, const std::vector<const char*>& Items, bool ForceOpen, const CCondition& Condition)
{
	m_Elements.emplace_back(new CMultiSelectToggleList(LONG_WIDTH, OBJECT_HEIGHT, OBJECT_GAP, szText, Items, ForceOpen, Condition), true);
	return m_Elements.size() - 1;
}

id_t CSubSection::AddNewTextNote(const char * szText, const CCondition & Condition)
{
	m_Elements.emplace_back(new CTextNote(szText, Condition), true);
	return m_Elements.size() - 1;
}

void CSubSection::Print()
{
	auto Current = m_RenderBounds;

	for (size_t i = 0; i < m_Elements.size(); i++)
	{
		if (!m_Elements[i]->IsVisible())
		{
			continue;
		}

		if (Current.top + m_Elements[i]->GetVerticalAdjustment() > (Current.bottom - 1))
		{
			return;
		}

		m_Elements[i]->SetRenderPosition({ Current.left + FONT_HEIGHT / 2, Current.top + FONT_HEIGHT / 2 });
		m_Elements[i]->Print();
		Current.top += m_Elements[i]->GetVerticalAdjustment() + OBJECT_GAP;
	}
}

void CSubSection::Control()
{
	for (auto& i : m_Elements)
	{
		i->Control();
	}
}

void CSubSection::Save(CConfig & cfg)
{
	for (auto& i : m_Elements)
	{
		i->Save(cfg);
	}
}

void CSubSection::Load(CConfig & cfg)
{
	for (auto& i : m_Elements)
	{
		i->Load(cfg);
	}
}

void CSubSection::Clear()
{
	for (auto& i : m_Elements)
	{
		if (i.m_bManaged)
		{
			delete i.m_Item;
			i.m_Item = nullptr;
		}
	}

	m_Elements.clear();
}

const std::vector<CManagedMenuItem<CBaseElement>>& CSubSection::GetElements() const
{
	return m_Elements;
}

const CBaseElement & CSubSection::GetElement(id_t nID) const
{
	return *(m_Elements[nID]());
}

std::vector<CManagedMenuItem<CBaseElement>>& CSubSection::GetElements() 
{
	return m_Elements;
}

CBaseElement & CSubSection::GetElement(id_t nID) 
{
	return *(m_Elements[nID]());
}

CCheckbox::CCheckbox(int w, int h, int gap, const char * Text, const CCondition & Condition)
	: CBaseElement(Text, Condition), m_Width(w), m_Height(h), m_Gap(gap), m_Value(false)
{
}

bool CCheckbox::Value() const
{
	return m_Value;
}

void CCheckbox::SetValue(bool New)
{
	m_Value = New;
}

void CCheckbox::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_Height };

	g_Render.FilledBox(m_RenderBounds, TERANY(m_Value, IMenu::GetSelectBrush(), IMenu::GetOutlineBrush()));
	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_RenderBounds.top, IMenu::GetTextColor(), m_RenderText);
}

void CCheckbox::Control()
{
	if (g_Menu.CursorInBounds(m_RenderBounds) && g_Menu.Mouse1Pressed())
	{
		m_Value = !m_Value;
	}
}

void CCheckbox::Save(CConfig & cfg)
{
	cfg.AddVar<bool>(m_RenderText, m_Value);
}

void CCheckbox::Load(CConfig & cfg)
{
	const auto var = cfg.GV<bool>(m_RenderText);
	if (var.m_ID != -1)
	{
		m_Value = var.m_Value;
	}
	else
	{
		m_Value = false;
	}
}

size_t CCheckbox::GetVerticalAdjustment() const
{
	return m_Height;
}

CButton::CButton(int w, int h, const char * Text, const std::function<void(void)>& Callback, const CCondition & Condition)
	: CBaseElement(Text, Condition), m_Width(w), m_Height(h), m_CallBack(Callback)
{
}

const std::function<void(void)>& CButton::CallBack() const
{
	return m_CallBack;
}

void CButton::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_Height };

	g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_Height, 1, IMenu::GetOutlineBrush());
	g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), m_RenderText);
}

void CButton::Control()
{
	if (g_Menu.CursorInBounds(m_RenderBounds) && g_Menu.Mouse1Pressed())
	{
		m_CallBack();
	}
}

size_t CButton::GetVerticalAdjustment() const
{
	return m_Height;
}

void CButton::Save(CConfig &)
{
	;
}

void CButton::Load(CConfig &)
{
	;
}

template<typename T>
CManagedMenuItem<T>::CManagedMenuItem(T * Item, bool bManaged) :
	m_Item(Item), m_bManaged(bManaged)
{
	;
}

template<typename T>
CManagedMenuItem<T>::~CManagedMenuItem()
{
	if (m_bManaged)
	{
		//delete m_Item;
	}
}

template<typename T>
T * CManagedMenuItem<T>::operator()()
{
	return m_Item;
}

template<typename T>
const T * CManagedMenuItem<T>::operator()() const
{
	return m_Item;
}

template<typename T>
T * CManagedMenuItem<T>::operator->()
{
	return (*this)();
}

template<typename T>
const T * CManagedMenuItem<T>::operator->() const
{
	return (*this)();
}

CBaseElement::CBaseElement(const char * RenderText, const CCondition& Condition)
	: CBaseControlable({}, RenderText, Condition)
{
}

void CBaseElement::SetRenderPosition(const POINT & New)
{
	m_CurrentRenderPosition = New;
}

CCondition::CCondition(const std::function<bool(void)>& Condition)
	: m_Condition(Condition)
{
}

CCondition & CCondition::operator=(const std::function<bool(void)>& New)
{
	SetNewTarget(New);
	return *this;
}

bool CCondition::PassesCondition() const
{
	return m_Condition();
}

void CCondition::SetNewTarget(const std::function<bool(void)>& New)
{
	m_Condition = New;
}

CUniSelect::CUniSelect(int w, int hsingle, int gap, const char * Text, const std::vector<const char*>& Items, const CCondition & Condition) :
	CBaseElement(Text, Condition), m_Width(w), m_HeightSingle(hsingle), m_Gap(gap), m_bIsOpen(false), m_Items(Items), m_Value(0)
{
}

size_t CUniSelect::Value() const
{
	return m_Value;
}

void CUniSelect::SetValue(size_t New)
{
	m_Value = New;
}

id_t CUniSelect::AddItem(const char * text)
{
	m_Items.emplace_back(text);
	return m_Items.size() - 1;
}

void CUniSelect::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_HeightSingle };

	g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_HeightSingle, 1, IMenu::GetOutlineBrush());
	g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), m_Items[m_Value]);

	if (m_bIsOpen)
	{
		for (size_t i = 0; i < m_Items.size(); i++)
		{
			m_RenderBounds.top += m_HeightSingle; m_RenderBounds.bottom += m_HeightSingle;

			if (i == m_Value)
			{
				g_Render.FilledBox(m_RenderBounds, IMenu::GetSelectBrush());
			}

			g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_HeightSingle, 1, IMenu::GetOutlineBrush());
			g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), m_Items[i]);
		}
	}

	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_CurrentRenderPosition.y, IMenu::GetTextColor(), m_RenderText);
}

void CUniSelect::Control()
{
	RECT Current = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_HeightSingle };

	if (g_Menu.CursorInBounds(Current) && g_Menu.Mouse1Pressed())
	{
		m_bIsOpen = !m_bIsOpen;
		return;
	}

	if (m_bIsOpen)
	{
		for (size_t i = 0; i < m_Items.size(); i++)
		{
			Current.top += m_HeightSingle; Current.bottom += m_HeightSingle;
			if (g_Menu.CursorInBounds(Current) && g_Menu.Mouse1Pressed())
			{
				m_Value = i;
				return;
			}
		}
	}
}

size_t CUniSelect::GetVerticalAdjustment() const
{
	return TERANY(m_bIsOpen, (m_Items.size() + 1) * m_HeightSingle, m_HeightSingle);
}

void CUniSelect::Save(CConfig & cfg)
{
	cfg.AddVar<int>(m_RenderText, m_Value);
}

void CUniSelect::Load(CConfig & cfg)
{
	const auto var = cfg.GV<int>(m_RenderText);
	m_Value = var.m_Value;

	if (m_Value < 0 || m_Value >= m_Items.size())
	{
		m_Value = 0;
	}
}

CKeySelect::CKeySelect(int w, int h, int gap, const char * Text, const CCondition & Condition)
	: CBaseElement(Text, Condition), m_Width(w), m_Height(h), m_Gap(gap), m_Value(0), m_bWaitingOnValue(false),
	m_pShouldToggle(new CCheckbox(SHORT_WIDTH, OBJECT_HEIGHT, OBJECT_GAP, (std::string(m_RenderText) + std::string("_toggle")).c_str(), M_C(this, this->m_Value != 0)))
{
}

key_t CKeySelect::Value() const
{
	return m_Value;
}

void CKeySelect::SetValue(key_t New)
{
	m_Value = New;
}

bool CKeySelect::Toggle() const
{
	return m_pShouldToggle->Value();
}

void CKeySelect::SetToggle(bool New)
{
	m_pShouldToggle->SetValue(New);
}

void CKeySelect::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_Height };

	g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_Height, 1, IMenu::GetOutlineBrush());
	g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), TERANY(m_bWaitingOnValue, "press_a_key", IMenu::GetKeyNames()[m_Value]));
	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_RenderBounds.top, IMenu::GetTextColor(), m_RenderText);

	if (m_pShouldToggle->IsVisible())
	{
		m_pShouldToggle->SetRenderPosition({ m_RenderBounds.left, m_RenderBounds.bottom + OBJECT_GAP });
		m_pShouldToggle->Print();
	}
}

void CKeySelect::Control()
{
	if (g_Menu.CursorInBounds(m_RenderBounds) && g_Menu.Mouse1Pressed())
	{
		m_bWaitingOnValue = !m_bWaitingOnValue;
	}

	if (m_bWaitingOnValue)
	{
		for (key_t i = VK_LBUTTON + 1; i < IMenu::GetKeyNames().size(); i++)
		{
			if (g_Input.KeyDown(i))
			{
				if (i == VK_ESCAPE)
				{
					m_Value = 0x0;
				}
				else
				{
					if (i == VK_INSERT)
					{
						m_Value = VK_LBUTTON;
					}
					else
					{
						m_Value = i;
					}
				}

				m_bWaitingOnValue = false;
				return;
			}
		}
	}

	if (m_pShouldToggle->IsVisible())
	{
		m_pShouldToggle->Control();
	}
}

size_t CKeySelect::GetVerticalAdjustment() const
{
	return TERANY(m_pShouldToggle->IsVisible(), m_Height + OBJECT_GAP + m_pShouldToggle->GetVerticalAdjustment(), m_Height);
}

void CKeySelect::Save(CConfig & cfg)
{
	cfg.AddVar<key_t>(m_RenderText, m_Value);
	m_pShouldToggle->Save(cfg);
}

void CKeySelect::Load(CConfig & cfg)
{
	const auto var = cfg.GV<key_t>(m_RenderText);
	if (var.m_ID != -1)
	{
		m_Value = var.m_Value;
	}
	else
	{
		m_Value = 0;
	}

	m_pShouldToggle->Load(cfg);
}

CMultiSelect::CMultiSelect(int w, int hsingle, int gap, const char * Text, const std::vector<const char*>& Items, const CCondition & Condition)
	: CBaseElement(Text, Condition), m_Width(w), m_HeightSingle(hsingle), m_Gap(gap), m_bIsOpen(false), m_Items(Items), m_SelectedItems()
{
}

const std::vector<size_t>& CMultiSelect::Value() const
{
	return m_SelectedItems;
}

id_t CMultiSelect::AddItem(const char * text)
{
	m_Items.emplace_back(text);
	return m_Items.size() - 1;
}

void CMultiSelect::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_HeightSingle };

	g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_HeightSingle, 1, IMenu::GetOutlineBrush());
	sprintf(buf, TERANY(m_SelectedItems.size() == 1, "%d Item", "%d Items"), m_SelectedItems.size());
	g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), buf);

	if (m_bIsOpen)
	{
		const auto IsSelected = [this](size_t index) -> bool
		{
			for (size_t i = 0; i < m_SelectedItems.size(); i++)
			{
				if (m_SelectedItems[i] == index)
				{
					return true;
				}
			}

			return false;
		};

		for (size_t i = 0; i < m_Items.size(); i++)
		{
			m_RenderBounds.top += m_HeightSingle; m_RenderBounds.bottom += m_HeightSingle;

			if (IsSelected(i))
			{
				g_Render.FilledBox(m_RenderBounds, IMenu::GetSelectBrush());
			}

			g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_HeightSingle, 1, IMenu::GetOutlineBrush());
			g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), m_Items[i]);
		}
	}

	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_CurrentRenderPosition.y, IMenu::GetTextColor(), m_RenderText);
}

void CMultiSelect::Control()
{
	RECT Current = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_HeightSingle };

	if (g_Menu.CursorInBounds(Current) && g_Menu.Mouse1Pressed())
	{
		m_bIsOpen = !m_bIsOpen;
		return;
	}

	if (m_bIsOpen)
	{
		const auto Exists = [this](size_t& index) -> bool
		{
			for (size_t i = 0; i < m_SelectedItems.size(); i++)
			{
				if (m_SelectedItems[i] == index)
				{
					index = i;
					return true;
				}
			}

			return false;
		};

		for (size_t i = 0; i < m_Items.size(); i++)
		{
			Current.top += m_HeightSingle; Current.bottom += m_HeightSingle;
			if (g_Menu.CursorInBounds(Current) && g_Menu.Mouse1Pressed())
			{
				if (Exists(i))
				{
					m_SelectedItems.erase(m_SelectedItems.begin() + i);
				}
				else
				{
					m_SelectedItems.emplace_back(i);
				}

				return;
			}
		}
	}
}

size_t CMultiSelect::GetVerticalAdjustment() const
{
	return TERANY(m_bIsOpen, m_HeightSingle * (m_Items.size() + 1), m_HeightSingle);
}

void CMultiSelect::Save(CConfig & cfg)
{
	const std::string render = m_RenderText;
	const std::string under = "_";

	for (size_t i = 0; i < m_SelectedItems.size(); i++)
	{
		cfg.AddVar<int>((render + under + std::to_string(i)).c_str(), m_SelectedItems[i]);
	}
}

void CMultiSelect::Load(CConfig & cfg)
{
	m_SelectedItems.clear();

	const std::string render = m_RenderText;
	const std::string under = "_";

	for (size_t i = 0; i < m_Items.size(); i++)
	{
		const auto var = cfg.GV<int>((render + under + std::to_string(i)).c_str());
		if (var.m_ID != -1)
		{
			m_SelectedItems.emplace_back(var.m_Value);
		}
	}
}

CMultiSelectToggleList::CMultiSelectToggleList(int w, int hsingle, int gap, const char * Text, const std::vector<const char*>& Items, bool ForceOpen, const CCondition & Condition)
	: CBaseElement(Text, Condition), m_Width(w), m_HeightSingle(hsingle), m_Gap(gap), m_bIsOpen(false), m_bForceOpen(ForceOpen)
{
	for (size_t i = 0; i < Items.size(); i++)
	{
		m_Items.emplace_back(Items[i], false);
	}
}

const std::vector<CMultiSelectToggleList::Item_t>& CMultiSelectToggleList::Value() const
{
	return m_Items;
}

id_t CMultiSelectToggleList::AddItem(const char * text)
{
	m_Items.emplace_back(text, false);
	return m_Items.size() - 1;
}

void CMultiSelectToggleList::Print()
{
	if (m_bForceOpen)
	{
		m_bIsOpen = true;
	}

	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_HeightSingle };

	const auto GetSelectedItemCount = [this]() -> size_t
	{
		size_t count = 0;
		for (size_t i = 0; i < m_Items.size(); i++)
		{
			if (m_Items[i].m_bSelected)
			{
				count++;
			}
		}

		return count;
	};

	const auto SelectedItemCount = GetSelectedItemCount();
	g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_HeightSingle, 1, IMenu::GetOutlineBrush());
	sprintf(buf, TERANY(SelectedItemCount == 1, "%d item", "%d items"), SelectedItemCount);
	g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), buf);

	if (m_bIsOpen)
	{
		for (size_t i = 0; i < m_Items.size(); i++)
		{
			m_RenderBounds.top += m_HeightSingle; m_RenderBounds.bottom += m_HeightSingle;

			if (m_Items[i].m_bSelected)
			{
				g_Render.FilledBox(m_RenderBounds, IMenu::GetSelectBrush());
			}

			g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_HeightSingle, 1, IMenu::GetOutlineBrush());
			g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), m_Items[i].m_szText);
		}
	}

	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_CurrentRenderPosition.y, IMenu::GetTextColor(), m_RenderText);
}

void CMultiSelectToggleList::Control()
{
	if (m_bForceOpen)
	{
		m_bIsOpen = true;
	}

	RECT Current = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_HeightSingle };

	if (g_Menu.CursorInBounds(Current) && g_Menu.Mouse1Pressed() && !m_bForceOpen)
	{
		m_bIsOpen = !m_bIsOpen;
		return;
	}

	if (m_bIsOpen)
	{
		for (size_t i = 0; i < m_Items.size(); i++)
		{
			Current.top += m_HeightSingle; Current.bottom += m_HeightSingle;
			if (g_Menu.CursorInBounds(Current) && g_Menu.Mouse1Pressed())
			{
				m_Items[i].m_bSelected = !m_Items[i].m_bSelected;
				return;
			}
		}
	}
}

size_t CMultiSelectToggleList::GetVerticalAdjustment() const
{
	return TERANY(m_bIsOpen, m_HeightSingle * (m_Items.size() + 1), m_HeightSingle);
}

void CMultiSelectToggleList::Save(CConfig & cfg)
{
	const std::string render = m_RenderText;
	const std::string under = "_";

	for (size_t i = 0; i < m_Items.size(); i++)
	{
		cfg.AddVar<bool>((render + under + m_Items[i].m_szText).c_str(), m_Items[i].m_bSelected);
	}
}

void CMultiSelectToggleList::Load(CConfig & cfg)
{
	const std::string render = m_RenderText;
	const std::string under = "_";

	for (size_t i = 0; i < m_Items.size(); i++)
	{
		m_Items[i].m_bSelected = cfg.GV<bool>((render + under + m_Items[i].m_szText).c_str()).m_Value;
	}
}

CColorSelect::CColorSelect(int w, int h, int gap, const char * Text, const CCondition & Condition)
	: CBaseElement(Text, Condition), m_Width(w), m_Height(h), m_Gap(gap), m_bOpen(false)
{
	memset(buf, 0, BUFFER_SIZE);
	const std::string render = m_RenderText;

	m_pRSlider = new CSlider<unsigned char>(LONG_WIDTH, m_Height, OBJECT_GAP, (render + "_r").c_str(), { 255, "" }, { 0, "" });
	m_pGSlider = new CSlider<unsigned char>(LONG_WIDTH, m_Height, OBJECT_GAP, (render + "_g").c_str(), { 255, "" }, { 0, "" });
	m_pBSlider = new CSlider<unsigned char>(LONG_WIDTH, m_Height, OBJECT_GAP, (render + "_b").c_str(), { 255, "" }, { 0, "" });
}

CColorSelect::~CColorSelect()
{
	delete m_pRSlider;
	delete m_pGSlider;
	delete m_pBSlider;
}

CColor CColorSelect::Value() const
{
	return CColor(m_pRSlider->Value(), m_pGSlider->Value(), m_pBSlider->Value());
}

void CColorSelect::SetValue(const CColor & New)
{
	m_pRSlider->SetValue(New.r());
	m_pGSlider->SetValue(New.g());
	m_pBSlider->SetValue(New.b());
}

void CColorSelect::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_Height };

		HBRUSH v = CreateSolidBrush(Value().GetRawColor());
		g_Render.FilledBox(m_RenderBounds, v);
		DeleteObject(v);
	

	g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_Height, 1, IMenu::GetOutlineBrush());

	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_RenderBounds.top, IMenu::GetTextColor(), m_RenderText);
	auto Current = m_RenderBounds;

	if (m_bOpen)
	{
		Current.top += m_Height; Current.bottom += m_Height;

		m_pRSlider->SetRenderPosition({ Current.left, Current.top });
		m_pRSlider->Print();
		Current.top += m_pRSlider->GetVerticalAdjustment(); Current.bottom += m_pRSlider->GetVerticalAdjustment();

		m_pGSlider->SetRenderPosition({ Current.left, Current.top });
		m_pGSlider->Print();
		Current.top += m_pGSlider->GetVerticalAdjustment(); Current.bottom += m_pGSlider->GetVerticalAdjustment();

		m_pBSlider->SetRenderPosition({ Current.left, Current.top });
		m_pBSlider->Print();
		Current.top += m_pBSlider->GetVerticalAdjustment(); Current.bottom += m_pBSlider->GetVerticalAdjustment();
	}
}

void CColorSelect::Control()
{
	if (g_Menu.CursorInBounds(m_RenderBounds) && g_Menu.Mouse1Pressed())
	{
		m_bOpen = !m_bOpen;
	}

	if (m_bOpen)
	{
		m_pRSlider->Control();
		m_pGSlider->Control();
		m_pBSlider->Control();
	}
}

size_t CColorSelect::GetVerticalAdjustment() const
{
	return TERANY(m_bOpen, m_Height + m_pRSlider->GetVerticalAdjustment() + m_pGSlider->GetVerticalAdjustment() + m_pBSlider->GetVerticalAdjustment(), m_Height);
}

void CColorSelect::Save(CConfig & cfg)
{
	m_pRSlider->Save(cfg);
	m_pGSlider->Save(cfg);
	m_pBSlider->Save(cfg);
}

void CColorSelect::Load(CConfig & cfg)
{
	m_pRSlider->Load(cfg);
	m_pGSlider->Load(cfg);
	m_pBSlider->Load(cfg);
}

bool IMenu::DragMenu()
{
	if (!m_bIsDragging)
	{
		m_LastCursor = g_Menu.GetCursorPosition();
	}

	RECT Bounds = { m_RenderBounds.left - TERANY(m_bIsDragging, 100, 0), m_RenderBounds.top - TERANY(m_bIsDragging, 100, 0), 
		m_RenderBounds.right + TERANY(m_bIsDragging, 100, 0), m_RenderBounds.top + FONT_HEIGHT + 1 + TERANY(m_bIsDragging, 100, 0) };
	m_bIsDragging = g_Menu.CursorInBounds(Bounds) && m_bMouse1Down;

	if (m_bIsDragging)
	{
		const POINT Delta = { g_Menu.GetCursorPosition().x - m_LastCursor.x, g_Menu.GetCursorPosition().y - m_LastCursor.y };
		m_RenderBounds.left += Delta.x; m_RenderBounds.right += Delta.x;
		m_RenderBounds.bottom += Delta.y; m_RenderBounds.top += Delta.y;
	}

	m_LastCursor = g_Menu.GetCursorPosition();
	return m_bIsDragging;
}

IMenu::IMenu(const RECT & Bounds, const char * RenderText, const CCondition & Condition)
	: CBaseControlable(Bounds, RenderText, Condition), m_bVisible(false), m_bIsDragging(false)
{
	m_Buttons.emplace_back(new CButton(LONG_WIDTH, OBJECT_HEIGHT, "quit", []() -> void { g_Engine.ForceShutDown(); }), true);
}

id_t IMenu::AddWindow(CWindow * W, bool bTransferOwnerShip)
{
	m_Windows.emplace_back(W, bTransferOwnerShip);
	m_Buttons.emplace_back(new CButton(LONG_WIDTH, OBJECT_HEIGHT, W->GetRenderText(), [W]() -> void { W->SetVisible(!W->GetVisible()); }, W->GetCondition()), true);
	return m_Windows.size() - 1;
}

id_t IMenu::AddNewWindow(const RECT & Bounds, const char * RenderText, const CCondition & Condition)
{
	m_Windows.emplace_back(new CWindow(Bounds, RenderText, Condition), true);
	CWindow* const ptr = m_Windows[m_Windows.size() - 1]();
	m_Buttons.emplace_back(new CButton(LONG_WIDTH, OBJECT_HEIGHT, ptr->GetRenderText(), [this, ptr]() -> void { ptr->SetVisible(!ptr->GetVisible()); }), true);
	return m_Windows.size() - 1;
}

id_t IMenu::AddButton(CButton * B, bool bTransferOwnerShip)
{
	m_Buttons.emplace_back(B, bTransferOwnerShip);
	return m_Buttons.size() - 1;
}

id_t IMenu::AddNewButton(int w, int h, const char * Text, const std::function<void(void)>& Callback, const CCondition & Condition)
{
	m_Buttons.emplace_back(new CButton(w, h, Text, Callback, Condition), true);
	return m_Buttons.size() - 1;
}

void IMenu::Print()
{
	static std::string str;
	static int length;
	{
		static bool binit = false;

		if (!binit)
		{
			std::string d(__DATE__);
			for (auto& i : d)
			{
				if (std::isalpha(i))
				{
					i = std::tolower(i);
				}
				else if (i == ' ')
				{
					i = '_';
				}
			}
			str = std::string("z1: ") + d;
		}

		length = g_Render.GetTextWidth(str.c_str());
	}
	g_Render.FilledBox(1, 1, length + 1, FONT_HEIGHT, IMenu::GetBackGroundBrush());
	g_Render.BorderedBox(0, 0, length + 2, FONT_HEIGHT + 1, 1, IMenu::GetTextBrush());
	g_Render.StringNotCentred(2, 1, IMenu::GetTextColor(), str.c_str());

	int iDrawn = 1;
	constexpr int iJump = FONT_HEIGHT + 1;
	for (size_t i = 0; i < m_szDebugMessages.size(); i++)
	{
		if (!m_szDebugMessages[i].IsValid())
		{
			m_szDebugMessages.erase(m_szDebugMessages.begin() + i);
			continue;
		}

		const int iLength = g_Render.GetTextWidth(m_szDebugMessages[i].szGetMessage());
		g_Render.FilledBox(1, 1 + iJump * iDrawn, iLength + 1, FONT_HEIGHT, IMenu::GetBackGroundBrush());
		g_Render.BorderedBox(0, iJump * iDrawn, iLength + 2, FONT_HEIGHT + 1, 1, IMenu::GetTextBrush());
		g_Render.StringNotCentred(2, 1 + iJump * iDrawn, IMenu::GetTextColor(), m_szDebugMessages[i].szGetMessage());
		iDrawn++;
	}

	if (!IsVisible())
	{
		return;
	}
	else
	{
		g_Local.SV<float>(CPlayer::IO::HealthShotBoostExpirationTime, g_GlobalVars.m_flCurTime + 2.f);
	}

	RECT Current = m_RenderBounds;
	g_Render.BorderedBox(Current.left - 1, Current.top - 1, Current.right - Current.left + 1, Current.bottom - Current.top + 1, 1, IMenu::GetTextBrush());
	g_Render.FilledBox(Current, IMenu::GetBackGroundBrush());
	Current.top += 1;
	g_Render.StringCentred((Current.left + Current.right) / 2, Current.top, IMenu::GetTextColor(), m_RenderText);
	Current.top += FONT_HEIGHT + 1;
	g_Render.Line(Current.left, Current.top, Current.right, Current.top, 1, IMenu::GetTextColor());
	Current.top += 1 + OBJECT_GAP;

	for (auto& i : m_Buttons)
	{
		if (!i()->IsVisible())
		{
			continue;
		}

		i->SetRenderPosition({ Current.left + LONG_WIDTH / 4, Current.top });
		i->Print();
		Current.top += i->GetVerticalAdjustment() + OBJECT_GAP;
	}

	for (auto& i : m_Windows)
	{
		i->Print();
	}
}

void IMenu::Control()
{
	const auto End = [&]() -> void
	{
		for (auto& i : m_Windows)
		{
			i->OnFailPrint();
		}
	};

	if (g_Input.KeyPressed(VK_DELETE))
	{
		m_bVisible = !m_bVisible;
	}

	m_bMouse1Pressed = g_Input.KeyPressed(VK_LBUTTON);
	m_bMouse1Down = g_Input.KeyDown(VK_LBUTTON);

	if (!IsVisible())
	{
		m_bIsDragging = false;
		End();
		return;
	}

	GetCursorPos(&m_CursorPosition);

	if (DragMenu())
	{
		End();
		return;
	}

	for (auto& i : m_Buttons)
	{
		if (!i()->IsVisible())
		{
			continue;
		}

		i->Control();
	}

	for (auto& i : m_Windows)
	{
		i->Control();
	}
}

bool IMenu::IsVisible() const
{
	return m_bVisible && m_Condition.PassesCondition();
}

void IMenu::Save(ConfigFile_t Reason)
{
	CConfig cfg(IMenu::GetConfigNames()[(int)Reason], ConfigReason::Save);

	for (auto& i : m_Windows)
	{
		i->Save(cfg);
	}
}

void IMenu::Load(ConfigFile_t Reason)
{
	CConfig cfg(IMenu::GetConfigNames()[(int)Reason], ConfigReason::Load);
	g_Local.SV<float>(CPlayer::IO::HealthShotBoostExpirationTime, g_GlobalVars.m_flCurTime + 10.f);

	for (auto& i : m_Windows)
	{
		i->Load(cfg);
	}
}

bool IMenu::Mouse1Pressed() const
{
	return m_bMouse1Pressed;
}

bool IMenu::Mouse1Down() const
{
	return m_bMouse1Down;
}

bool IMenu::CursorInBounds(const RECT & Bounds) const
{
	return (m_CursorPosition.x >= Bounds.left && m_CursorPosition.y >= Bounds.top &&
		m_CursorPosition.x <= Bounds.right && m_CursorPosition.y <= Bounds.bottom);
}

const std::vector<CManagedMenuItem<CWindow>>& IMenu::GetWindows() const
{
	return m_Windows;
}

const CWindow & IMenu::GetWindow(id_t nID) const
{
	return *(m_Windows[nID]());
}

std::vector<CManagedMenuItem<CWindow>>& IMenu::GetWindows() 
{
	return m_Windows;
}

CWindow & IMenu::GetWindow(id_t nID)
{
	return *(m_Windows[nID]());
}

const std::vector<CManagedMenuItem<CButton>>& IMenu::GetButtons() const
{
	return m_Buttons;
}

const CButton & IMenu::GetButton(id_t nID) const
{
	return *(m_Buttons[nID]());
}

std::vector<CManagedMenuItem<CButton>>& IMenu::GetButtons() 
{
	return m_Buttons;
}

CButton & IMenu::GetButton(id_t nID) 
{
	return *(m_Buttons[nID]());
}

CColor IMenu::GetOutlineColor()
{
	return g_SharedCon.m_pOutlineColor->Value();
}

const HBRUSH & IMenu::GetOutlineBrush()
{
	static CColor Last = IMenu::GetOutlineColor();
	static HBRUSH ret = CreateSolidBrush(Last.GetRawColor());
	if (Last != IMenu::GetOutlineColor())
	{
		DeleteObject(ret);
		Last = IMenu::GetOutlineColor();
		ret = CreateSolidBrush(Last.GetRawColor());
	}

	return ret;
}

CColor IMenu::GetSelectColor()
{
	return g_SharedCon.m_pSelectColor->Value();
}

const HBRUSH & IMenu::GetSelectBrush()
{
	static CColor Last = IMenu::GetSelectColor();
	static HBRUSH ret = CreateSolidBrush(Last.GetRawColor());
	if (Last != IMenu::GetSelectColor())
	{
		DeleteObject(ret);
		Last = IMenu::GetSelectColor();
		ret = CreateSolidBrush(Last.GetRawColor());
	}

	return ret;
}

CColor IMenu::GetBackGroundColor()
{
	return g_SharedCon.m_pBackGroundColor->Value();
}

const HBRUSH & IMenu::GetBackGroundBrush()
{
	static CColor Last = IMenu::GetBackGroundColor();
	static HBRUSH ret = CreateSolidBrush(Last.GetRawColor());
	if (Last != IMenu::GetBackGroundColor())
	{
		DeleteObject(ret);
		Last = IMenu::GetBackGroundColor();
		ret = CreateSolidBrush(Last.GetRawColor());
	}

	return ret;
}

CColor IMenu::GetTextColor()
{
	return g_SharedCon.m_pTextColor->Value();
}

const HBRUSH & IMenu::GetTextBrush()
{
	static CColor Last = IMenu::GetTextColor();
	static HBRUSH ret = CreateSolidBrush(Last.GetRawColor());
	if (Last != IMenu::GetTextColor())
	{
		DeleteObject(ret);
		Last = IMenu::GetTextColor();
		ret = CreateSolidBrush(Last.GetRawColor());
	}

	return ret;
}

const std::array<const char*, 6>& IMenu::GetConfigNames()
{
	static std::array<const char*, 6> szConfigs = { "level_0", "level_1", "level_2", "level_3", "level_4", "level_5" };
	return szConfigs;
}

const std::array<const char*, 124>& IMenu::GetKeyNames()
{
	static std::array<const char*, 124> szStrings =
	{
		"unbound",
		"mouse 1",
		"mouse 2",
		"cancel",
		"middle mouse",
		"mouse 4",
		"mouse 5",
		"",
		"backspace",
		"tab",
		"",
		"",
		"clear",
		"enter",
		"",
		"",
		"shift",
		"control",
		"alt",
		"pause",
		"caps",
		"",
		"",
		"",
		"",
		"",
		"",
		"escape",
		"",
		"",
		"",
		"",
		"space",
		"page up",
		"page down",
		"end",
		"home",
		"left",
		"up",
		"right",
		"down",
		"",
		"",
		"",
		"print",
		"insert",
		"delete",
		"",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"a",
		"b",
		"c",
		"d",
		"e",
		"f",
		"g",
		"h",
		"i",
		"j",
		"k",
		"l",
		"m",
		"n",
		"o",
		"p",
		"q",
		"r",
		"s",
		"t",
		"u",
		"v",
		"w",
		"x",
		"y",
		"z",
		"",
		"",
		"",
		"",
		"",
		"numpad 0",
		"numpad 1",
		"numpad 2",
		"numpad 3",
		"numpad 4",
		"numpad 5",
		"numpad 6",
		"numpad 7",
		"numpad 8",
		"numpad 9",
		"multiply",
		"add",
		"",
		"subtract",
		"decimal",
		"divide",
		"f1",
		"f2",
		"f3",
		"f4",
		"f5",
		"f6",
		"f7",
		"f8",
		"f9",
		"f10",
		"f11",
		"f12"
	};

	return szStrings;
}

constexpr int IMenu::GetKeyCount()
{
	return 124;
}

CTextNote::CTextNote(const char * Text, const CCondition & Condition) :
	CBaseElement(Text, Condition)
{
	;
}

void CTextNote::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + g_Render.GetTextWidth(m_RenderText), m_CurrentRenderPosition.y + FONT_HEIGHT };

	g_Render.StringNotCentred(m_CurrentRenderPosition.x, m_CurrentRenderPosition.y, IMenu::GetTextColor(), m_RenderText);
}

void CTextNote::Control()
{
	;
}

void CTextNote::Save(CConfig &)
{
	;
}

void CTextNote::Load(CConfig &)
{
	;
}

size_t CTextNote::GetVerticalAdjustment() const
{
	return FONT_HEIGHT;
}

void CCycle::Clamp()
{
	if (m_Value < 0)
	{
		m_Value = 0;
	}
	else if (m_Value >= m_Items.size())
	{
		m_Value = 0;
	}
}

CCycle::CCycle(int w, int h, int gap, const char * Text, const std::vector<const char*>& Items, const CCondition & Condition)
	: CBaseElement(Text, Condition), m_Width(w), m_Height(h), m_Gap(gap), m_Value(0), m_Items(Items)
{
}

size_t CCycle::Value() const
{
	return m_Value;
}

void CCycle::SetValue(size_t New)
{
	m_Value = New;
}

void CCycle::Print()
{
	m_RenderBounds = { m_CurrentRenderPosition.x, m_CurrentRenderPosition.y,
		m_CurrentRenderPosition.x + (LONG)m_Width, m_CurrentRenderPosition.y + (LONG)m_Height };

	g_Render.BorderedBox(m_RenderBounds.left, m_RenderBounds.top, m_Width, m_Height, 1, IMenu::GetOutlineBrush());
	g_Render.StringCentred((m_RenderBounds.left + m_RenderBounds.right) / 2, m_RenderBounds.top, IMenu::GetTextColor(), m_Items[m_Value]);
	g_Render.StringNotCentred(m_RenderBounds.right + m_Gap, m_RenderBounds.top, IMenu::GetTextColor(), m_RenderText);
}

void CCycle::Control()
{
	if (g_Menu.Mouse1Pressed() && g_Menu.CursorInBounds(m_RenderBounds))
	{
		m_Value++;
		Clamp();
	}
}
void CCycle::Save(CConfig & cfg)
{
	cfg.AddVar<int>(m_RenderText, m_Value);
}

void CCycle::Load(CConfig & cfg)
{
	const auto var = cfg.GV<int>(m_RenderText);
	if (var.m_ID != -1)
	{
		m_Value = var.m_Value;
	}
	else
	{
		m_Value = 0;
	}
}

size_t CCycle::GetVerticalAdjustment() const
{
	return m_Height;
}
