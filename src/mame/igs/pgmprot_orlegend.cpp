// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, iq_132
/***********************************************************************
 PGM ASIC3 PGM protection emulation

 this seems similar to the IGS025? Is the physical chip ASIC3, or is
 that just what the game calls it?

 Used by:

 Oriental Legend

 ***********************************************************************/

#include "emu.h"
#include "pgm.h"
#include "pgmprot_orlegend.h"

void pgm_asic3_state::asic3_compute_hold(int y, int z)
{
	const u16 old = m_asic3_hold;

	m_asic3_hold = ((old << 1) | (old >> 15));

	m_asic3_hold ^= 0x2bad;
	m_asic3_hold ^= BIT(z, y);
	m_asic3_hold ^= BIT(m_asic3_x, 2) << 10;
	m_asic3_hold ^= BIT(old, 5);

	switch (m_region->read()) // The mode is dependent on the region
	{
		case 0:
		case 1:
			m_asic3_hold ^= BIT(old, 10) ^ BIT(old, 8) ^ (BIT(m_asic3_x, 0) << 1) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 3) << 14);
		break;

		case 2:
			m_asic3_hold ^= BIT(old, 10) ^ BIT(old, 8) ^ (BIT(m_asic3_x, 0) << 4) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 3) << 12);
		break;

		case 3:
			m_asic3_hold ^= BIT(old,  7) ^ BIT(old, 6) ^ (BIT(m_asic3_x, 0) << 4) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 3) << 12);
		break;

		case 4: // orlegend111t
			m_asic3_hold ^= BIT(old,  7) ^ BIT(old, 6) ^ (BIT(m_asic3_x, 0) << 3) ^ (BIT(m_asic3_x, 1) << 8) ^ (BIT(m_asic3_x, 3) << 14);
		break;
	}
}
u16 pgm_asic3_state::pgm_asic3_r()
{
	switch (m_asic3_reg)
	{
		case 0x00: // region is supplied by the protection device
			return (m_asic3_latch[0] & 0xf7) | ((m_region->read() << 3) & 0x08);

		case 0x01:
			return m_asic3_latch[1];

		case 0x02: // region is supplied by the protection device
			return (m_asic3_latch[2] & 0x7f) | ((m_region->read() << 6) & 0x80);

		case 0x03:
			return bitswap<8>(m_asic3_hold, 5,2,9,7,10,13,12,15);

		// case $157674, expected return $157686
		case 0x20: return 0x49; // "IGS"
		case 0x21: return 0x47;
		case 0x22: return 0x53;

		case 0x24: return 0x41;
		case 0x25: return 0x41;
		case 0x26: return 0x7f;
		case 0x27: return 0x41;
		case 0x28: return 0x41;

		case 0x2a: return 0x3e;
		case 0x2b: return 0x41;
		case 0x2c: return 0x49;
		case 0x2d: return 0xf9;
		case 0x2e: return 0x0a;

		case 0x30: return 0x26;
		case 0x31: return 0x49;
		case 0x32: return 0x49;
		case 0x33: return 0x49;
		case 0x34: return 0x32;

	//  default:
	//       logerror("ASIC3 R: CMD %2.2X %s\n", m_asic3_reg, machine().describe_context());
	}

	return 0;
}

void pgm_asic3_state::pgm_asic3_w(offs_t offset, u16 data)
{
	if (offset == 0)
	{
		m_asic3_reg = data;
		return;
	}

	switch (m_asic3_reg)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			m_asic3_latch[m_asic3_reg] = data << 1;
		break;

	//  case 0x03: // move.w  #$88, $c0400e.l
	//  case 0x04: // move.w  #$84, $c0400e.l
	//  case 0x05: // move.w  #$A0, $c0400e.l
	//  break;

		case 0x40:
			m_asic3_hilo = (m_asic3_hilo << 8) | data;
		break;

		case 0x41: // Same as CMD 40. What is the purpose of writing data here again??
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		break;

		case 0x48:
		{
			m_asic3_x = 0;
			if ((m_asic3_hilo & 0x0090) == 0) m_asic3_x |= 0x01;
			if ((m_asic3_hilo & 0x0006) == 0) m_asic3_x |= 0x02;
			if ((m_asic3_hilo & 0x9000) == 0) m_asic3_x |= 0x04;
			if ((m_asic3_hilo & 0x0a00) == 0) m_asic3_x |= 0x08;
		}
		break;

	//  case 0x50: // move.w  #$50, $c0400e.l
	//  break;

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
			asic3_compute_hold(m_asic3_reg & 0x07, data);
		break;

		case 0xa0:
			m_asic3_hold = 0;
		break;

		default:
			logerror("ASIC3 W: CMD %2.2X DATA: %4.4x %s\n", m_asic3_reg, data, machine().describe_context());
	}
}

/* Oriental Legend INIT */

void pgm_asic3_state::init_orlegend()
{
	pgm_basic_init();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc04000, 0xc0400f, read16smo_delegate(*this, FUNC(pgm_asic3_state::pgm_asic3_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xc04000, 0xc0400f, write16sm_delegate(*this, FUNC(pgm_asic3_state::pgm_asic3_w)));

	m_asic3_reg = 0;
	m_asic3_latch[0] = 0;
	m_asic3_latch[1] = 0;
	m_asic3_latch[2] = 0;
	m_asic3_x = 0;
	m_asic3_hilo = 0;
	m_asic3_hold = 0;

	save_item(NAME(m_asic3_reg));
	save_item(NAME(m_asic3_latch));
	save_item(NAME(m_asic3_x));
	save_item(NAME(m_asic3_hilo));
	save_item(NAME(m_asic3_hold));
}


INPUT_PORTS_START( orlegend )
	PORT_INCLUDE ( pgm )

// 修改的 代码来源 (EKMAME) 
/***********************************************************************************************************************************************************************************************/
	PORT_MODIFY("P1P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("P1P2", 0x00F0, NOTEQUALS, 0x0020)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("P1P2", 0x00F0, NOTEQUALS, 0x0040)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("P1P2", 0x00F0, NOTEQUALS, 0x0080)
	PORT_BIT( 0x0060, IP_ACTIVE_LOW, IPT_BUTTON_AB ) PORT_PLAYER(1) PORT_NAME("P1 Button Combokey (Button 1 + Button 2)") PORT_CONDITION("P1P2", 0x00F0, NOTEQUALS, 0x0060)	
	PORT_BIT( 0x00E0, IP_ACTIVE_LOW, IPT_BUTTON_ABC ) PORT_PLAYER(1) PORT_NAME("P1 Button Combokey (Button 1 + Button 2 + Button 3)") PORT_CONDITION("P1P2", 0x00F0, NOTEQUALS, 0x00E0)	

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("P1P2", 0xF000, NOTEQUALS, 0x2000)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("P1P2", 0xF000, NOTEQUALS, 0x4000)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("P1P2", 0xF000, NOTEQUALS, 0x8000)
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_BUTTON_AB ) PORT_PLAYER(2) PORT_NAME("P2 Button Combokey (Button 1 + Button 2)") PORT_CONDITION("P1P2", 0xF000, NOTEQUALS, 0x6000)	
	PORT_BIT( 0xE000, IP_ACTIVE_LOW, IPT_BUTTON_ABC ) PORT_PLAYER(2) PORT_NAME("P2 Button Combokey (Button 1 + Button 2 + Button 3)") PORT_CONDITION("P1P2", 0xF000, NOTEQUALS, 0xE000)	
/***********************************************************************************************************************************************************************************************/

	PORT_MODIFY("Region")
	PORT_BIT(     0xfffc, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0001, "World (duplicate)" ) // again?
	PORT_CONFSETTING(      0x0002, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( China ) )
INPUT_PORTS_END

INPUT_PORTS_START( orlegendt )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_BIT(     0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, "Invalid 00?" )
	PORT_CONFSETTING(      0x0001, "Invalid 01?" )
	PORT_CONFSETTING(      0x0002, "Invalid 02?" )
	PORT_CONFSETTING(      0x0003, "Invalid 03?" )
	PORT_CONFSETTING(      0x0004, DEF_STR( Taiwan ) )
INPUT_PORTS_END


INPUT_PORTS_START( orlegendk )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_BIT(     0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0007, 0x0002, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, "Invalid 00?" )
	PORT_CONFSETTING(      0x0001, "Invalid 01?" )
	PORT_CONFSETTING(      0x0002, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0003, "Invalid 03?" )
	PORT_CONFSETTING(      0x0004, "Invalid 04?" )
INPUT_PORTS_END


void pgm_asic3_state::pgm_asic3(machine_config &config)
{
	pgmbase(config);
}
