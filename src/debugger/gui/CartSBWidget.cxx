//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2017 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "CartSB.hxx"
#include "PopUpWidget.hxx"
#include "CartSBWidget.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeSBWidget::CartridgeSBWidget(
      GuiObject* boss, const GUI::Font& lfont, const GUI::Font& nfont,
      int x, int y, int w, int h, CartridgeSB& cart)
  : CartDebugWidget(boss, lfont, nfont, x, y, w, h),
    myCart(cart)
{
  uInt32 size = myCart.mySize;

  VariantList items;
  ostringstream info, bank;
  info << "SB SUPERbanking, 32 or 64 4K banks\n"
       << "Hotspots are from $800 to $"
       << Common::Base::HEX2 << (0x800 + myCart.bankCount() - 1) << ", including\n"
       << "mirrors ($900, $A00, $B00, ...)\n"
       << "Startup bank = " << std::dec << cart.myStartBank << "\n";

  // Eventually, we should query this from the debugger/disassembler
  for(uInt32 i = 0, offset = 0xFFC, spot = 0x800; i < myCart.bankCount();
      ++i, offset += 0x1000, ++spot)
  {
    uInt16 start = (cart.myImage[offset+1] << 8) | cart.myImage[offset];
    start -= start % 0x1000;
    info << "Bank " << std::dec << i << " @ $" << Common::Base::HEX4 << start << " - "
         << "$" << (start + 0xFFF) << " (hotspot = $" << spot << ")\n";

    bank << std::dec << std::setw(2) << std::setfill(' ') << i << " ($"
         << Common::Base::HEX2 << spot << ")";
    VarList::push_back(items, bank.str());
    bank.str("");
  }

  int xpos = 10,
      ypos = addBaseInformation(size, "Fred X. Quimby", info.str()) + myLineHeight;

  myBank =
    new PopUpWidget(boss, _font, xpos, ypos-2, _font.getStringWidth("XX ($800) "),
                    myLineHeight, items, "Set bank ",
                    _font.getStringWidth("Set bank "), kBankChanged);
  myBank->setTarget(this);
  addFocusWidget(myBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeSBWidget::loadConfig()
{
  Debugger& dbg = instance().debugger();
  CartDebug& cart = dbg.cartDebug();
  const CartState& state = static_cast<const CartState&>(cart.getState());
  const CartState& oldstate = static_cast<const CartState&>(cart.getOldState());

  myBank->setSelectedIndex(myCart.getBank(), state.bank != oldstate.bank);

  CartDebugWidget::loadConfig();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeSBWidget::handleCommand(CommandSender* sender,
                                      int cmd, int data, int id)
{
  if(cmd == kBankChanged)
  {
    myCart.unlockBank();
    myCart.bank(myBank->getSelected());
    myCart.lockBank();
    invalidate();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string CartridgeSBWidget::bankState()
{
  ostringstream& buf = buffer();

  buf << "Bank = " << std::dec << myCart.getBank()
      << ", hotspot = $" << Common::Base::HEX2 << (myCart.getBank() + 0x800);

  return buf.str();
}
