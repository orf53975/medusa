#include "medusa/screen.hpp"
#include <algorithm>

MEDUSA_NAMESPACE_USE

Screen::Screen(Medusa& rCore, Printer& rPrinter, u32 Width, u32 Height, Address const& rAddress)
  : m_rCore(rCore)
  , m_rPrinter(rPrinter)
  , m_Width(Width), m_Height(Height)
  , m_xOffset(), m_yOffset()
{
  _Prepare(rAddress);
}

Cell* Screen::GetCellFromPosition(u32 xChar, u32 yChar)
{
  Address CellAddr;

  if (GetAddressFromPosition(CellAddr, xChar, yChar) == false)
    return nullptr;

  return m_rCore.GetCell(CellAddr);
}

Cell const* Screen::GetCellFromPosition(u32 xChar, u32 yChar) const
{
  Address CellAddr;

  if (GetAddressFromPosition(CellAddr, xChar, yChar) == false)
    return nullptr;

  return m_rCore.GetCell(CellAddr);
}

void Screen::GetDimension(u32& rWidth, u32& rHeight) const
{
  rWidth  = m_Width;
  rHeight = m_Height;
}

void Screen::Refresh(void)
{
  auto FirstAddr = *m_VisiblesAddresses.begin();
  _Prepare(FirstAddr);
}

void Screen::Resize(u32 Width, u32 Height)
{
  m_Width  = Width;
  m_Height = Height;
}

void Screen::Print(void)
{
  auto& rDatabase = m_rCore.GetDatabase();

  u32 LineNo;
  u32 yOffset = 0;

  for (auto itAddr = std::begin(m_VisiblesAddresses); itAddr != std::end(m_VisiblesAddresses);)
  {
    LineNo = m_rPrinter(*itAddr, m_xOffset, yOffset);
    if (LineNo == 0)
      return;
    yOffset += LineNo;
    while (LineNo--)
      ++itAddr;
  }
}

bool Screen::Scroll(s32 xOffset, s32 yOffset)
{
  u32 MaxNumberOfAddress = m_rCore.GetDatabase().GetNumberOfAddress();
  if (m_yOffset == MaxNumberOfAddress)
    return false;

  Address NewAddress = *m_VisiblesAddresses.begin();

  if (m_rCore.GetDatabase().MoveAddress(NewAddress, NewAddress, yOffset) == false)
    return false;

  _Prepare(NewAddress);
  m_xOffset += xOffset;
  m_yOffset += yOffset;

  if (m_yOffset > MaxNumberOfAddress)
  {
    m_yOffset = MaxNumberOfAddress;
    return false;
  }

  return true;
}

bool Screen::Move(u32 xPosition, u32 yPosition)
{
  Address NewAddress;
  if (yPosition != -1)
  {
    if (m_rCore.GetDatabase().ConvertPositionToAddress(yPosition, NewAddress) == false)
      return false;
    m_yOffset = yPosition;
    _Prepare(NewAddress);
  }

  if (xPosition != -1)
    m_xOffset = xPosition;

  return true;
}

void Screen::_Prepare(Address const& rAddress)
{
  auto const& rDatabase = m_rCore.GetDatabase();
  u32 NumberOfAddress = m_Height;
  Address CurrentAddress;

  if (m_VisiblesAddresses.empty() == false)
    m_VisiblesAddresses.erase(std::begin(m_VisiblesAddresses), std::end(m_VisiblesAddresses));

  if (NumberOfAddress == 0)
    return;

  if (rDatabase.GetNearestAddress(rAddress, CurrentAddress) == false)
    return;

  while (NumberOfAddress--)
  {
    u32 NumberOfLine = m_rPrinter.GetNumberOfLine(CurrentAddress);

    if (NumberOfLine == 0)
      continue;

    while (NumberOfLine--)
      m_VisiblesAddresses.push_back(CurrentAddress);

    if (rDatabase.GetNextAddress(CurrentAddress, CurrentAddress) == false)
      return;
  }
}

bool Screen::GoTo(Address const& rAddress)
{
  _Prepare(rAddress);
  return m_VisiblesAddresses.empty() == true ? false : true;
}

bool Screen::GetAddressFromPosition(Address& rAddress, u32 xPos, u32 yPos) const
{
  if (yPos >= m_VisiblesAddresses.size())
    return false;

  auto itAddr = m_VisiblesAddresses.begin();
  while (yPos--)
    ++itAddr;

  rAddress = *itAddr;
  return true;
}