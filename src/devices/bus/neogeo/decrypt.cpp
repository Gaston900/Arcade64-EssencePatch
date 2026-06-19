// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli,Gaston90
/***********************************************************************************************************

 Neo Geo cart emulation
 KOF 2002 Encrypted cart type cart type (CMC + PCM2 + Additional CPU encryption)
 PVC encrypted cart type (+ CMC + PCM2)
 Encrypted cart type  (CMC42 & CMC50)
 Crouching Tiger Hidden Dragon Bootleg cart type
 PCM2 encrypted cart type
 Misc. bootleg cart types (possibly to be split further at a later stage)

 ***********************************************************************************************************/


#include "emu.h"
#include "decrypt.h"

// ================================================================> NEOGEO_DECRYPT_CART_DEVICE

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_CART, neogeo_decrypt_cart_device, "neocart_decrypt", "Neo Geo DECRYPTED-Type Cart")


neogeo_decrypt_cart_device::neogeo_decrypt_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_kof2k2_prot(*this, "kof2002_prot"),
	m_prot(*this, "bootleg_prot"),
	m_cthd_prot(*this, "cthd_prot"),
	m_kof98_prot(*this, "kof98_prot"),
	m_kof2k3bl_prot(*this, "kof2k3bl_prot")
{
}

neogeo_decrypt_cart_device::neogeo_decrypt_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_decrypt_cart_device::device_start()
{
}

void neogeo_decrypt_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_decrypt_cart_device::device_add_mconfig(machine_config &config)
{
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
	NG_KOF2002_PROT(config, m_kof2k2_prot);
	NEOBOOT_PROT(config, m_prot);
	NG_CTHD_PROT(config, m_cthd_prot);
    NG_KOF98_PROT(config, m_kof98_prot);
    NG_KOF2K3BL_PROT(config, m_kof2k3bl_prot);
}

// ================================================================> NEOGEO_DECRYPT_PVC_CART_DEVICE

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_CART, neogeo_decrypt_pvc_cart_device, "neocart_pvcd", "Neo Geo PVC DECRYPTED Cart")


neogeo_decrypt_pvc_cart_device::neogeo_decrypt_pvc_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_prot(*this, "bootleg_prot"),
	m_cmc_prot(*this, "cmc_prot"),
	m_pcm2_prot(*this, "pcm2_prot"),
	m_pvc_prot(*this, "pvc_prot")
{
}

neogeo_decrypt_pvc_cart_device::neogeo_decrypt_pvc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_decrypt_pvc_cart_device(mconfig, NEOGEO_DECRYPT_PVC_CART, tag, owner, clock)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_decrypt_pvc_cart_device::device_start()
{
}

void neogeo_decrypt_pvc_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_decrypt_pvc_cart_device::device_add_mconfig(machine_config &config)
{
    NEOBOOT_PROT(config, m_prot);
	NG_CMC_PROT(config, m_cmc_prot);
	NG_PCM2_PROT(config, m_pcm2_prot);
	NG_PVC_PROT(config, m_pvc_prot);
}

// ================================================================> NEOGEO_SMA_DECRYPT_CART_DEVICE
//-------------------------------------------------
//  neogeo_sma_decrypt_cart_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NEOGEO_SMA_DECRYPT_CART,                 neogeo_sma_decrypt_cart_device,             "neocart_sma_decrypt",     "Neo Geo SMA DECRYPTED Cart")
DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_GAROUD_CART,          neogeo_decrypt_sma_garoud_cart_device,      "neocart_garoud",          "Neo Geo Garou Decrypted SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_GAROUHD_CART,         neogeo_decrypt_sma_garouhd_cart_device,     "neocart_garouhd",         "Neo Geo Garou Decrypted AES SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_KOF99D_CART,          neogeo_decrypt_sma_kof99d_cart_device,      "neocart_kof99d",          "Neo Geo KoF 99 Decrypted SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_KOF2000D_CART,        neogeo_decrypt_sma_kof2000d_cart_device,    "neocart_kof2000d",        "Neo Geo KoF 2000 Decrypted SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_KOF2000ND_CART,       neogeo_decrypt_sma_kof2000nd_cart_device,   "neocart_kof2000nd",       "Neo Geo KoF 2000 ND Decrypted SMA Cart")
DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_MSLUG3D_CART,         neogeo_decrypt_sma_mslug3d_cart_device,     "neocart_mslug3d",         "Neo Geo Metal Slug 3 Decrypted SMA Cart (green)")

neogeo_sma_decrypt_cart_device::neogeo_sma_decrypt_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_rom_device(mconfig, type, tag, owner, clock),
	m_sma_prot(*this, "sma_prot"),
	m_cmc_prot(*this, "cmc_prot")
{
}

neogeo_sma_decrypt_cart_device::neogeo_sma_decrypt_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock) :
	neogeo_sma_decrypt_cart_device(mconfig, NEOGEO_SMA_DECRYPT_CART, tag, owner, clock)
{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void neogeo_sma_decrypt_cart_device::device_start()
{
}

void neogeo_sma_decrypt_cart_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

void neogeo_sma_decrypt_cart_device::device_add_mconfig(machine_config &config)
{
	NG_SMA_PROT(config, m_sma_prot);
	NG_CMC_PROT(config, m_cmc_prot);
}

// ================================================================> NEOGEO_DECRYPT_CART_DEVICE

/*************************************************
 cthd2k3a
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_CTHD2K3A_CART, neogeo_decrypt_cthd2k3a_cart_device, "neocart_cthd2k3a", "Neo Geo Crouching Tiger Hidden Dragon 2003 bootleg set 2 Decrypted Cart")

neogeo_decrypt_cthd2k3a_cart_device::neogeo_decrypt_cthd2k3a_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_CTHD2K3A_CART, tag, owner, clock)
{
}

void neogeo_decrypt_cthd2k3a_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	uint8_t nBank[] = {
		0x06, 0x02, 0x04, 0x05, 0x01, 0x03, 0x00, 0x07,
		0x27, 0x0E, 0x1C, 0x15, 0x1B, 0x17, 0x0A, 0x0F,
		0x16, 0x14, 0x23, 0x0B, 0x22, 0x26, 0x08, 0x24,
		0x21, 0x13, 0x1A, 0x0C, 0x19, 0x1D, 0x25, 0x10,
		0x09, 0x20, 0x18, 0x1F, 0x1E, 0x12, 0x0D, 0x11
	};

	uint8_t *src = cpuregion;
	std::vector<uint8_t> dst( cpuregion_size );

	for (int i = 0; i < 0x500000 / 0x20000; i++)
		memcpy (&dst[i * 0x20000], src + nBank[i] * 0x20000, 0x20000);

	memcpy (src, &dst[0], 0x500000);

    m_cthd_prot->decrypt_cthd2003(spr_region, spr_region_size, audiocpu_region, audio_region_size, fix_region, fix_region_size);
}

/*************************************************
 jckeygpd
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_JOCKEYGPD_CART, neogeo_decrypt_jockeygpd_cart_device, "neocart_jockeygpd", "Neo Geo Jockey GP Decrypted CMC50 Cart")

neogeo_decrypt_jockeygpd_cart_device::neogeo_decrypt_jockeygpd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_JOCKEYGPD_CART, tag, owner, clock),
	m_nvram(*this, "nvram")
{
}

void neogeo_decrypt_jockeygpd_cart_device::device_start()
{
	m_ram = make_unique_clear<uint16_t[]>(0x2000/2);
	m_nvram->set_base(m_ram.get(), 0x2000);
	save_pointer(NAME(m_ram), 0x2000/2);
}

void neogeo_decrypt_jockeygpd_cart_device::device_add_mconfig(machine_config &config)
{
	neogeo_decrypt_cart_device::device_add_mconfig(config);
	NVRAM(config, m_nvram);
}

/*************************************************
 kof96ep
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF96EP_CART, neogeo_decrypt_kof96ep_cart_device, "neocart_kof96ep", "Neo Geo KoF 96 bootleg Cart")

neogeo_decrypt_kof96ep_cart_device::neogeo_decrypt_kof96ep_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KOF96EP_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kof96ep_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	int i,j;
	uint8_t *rom = cpuregion;
	for ( i=0; i < 0x080000; i++ )
	{
		j=i+0x300000;
		if (rom[j] - rom[i] == 8) rom[j]=rom[i];
	}
	memcpy(rom, rom+0x300000, 0x080000);
}

/*************************************************
 kof97pla
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF97PLA_CART, neogeo_decrypt_kof97pla_cart_device, "neocart_kof97pla", "Neo Geo KoF 97 Plus bootleg Cart")

neogeo_decrypt_kof97pla_cart_device::neogeo_decrypt_kof97pla_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KOF97PLA_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kof97pla_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}

/*************************************************
 kof2002d
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF2002D_CART, neogeo_decrypt_kof2002d_cart_device, "neocart_kof2002d", "Neo Geo KoF 2002 Decrypted Cart")

neogeo_decrypt_kof2002d_cart_device::neogeo_decrypt_kof2002d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KOF2002D_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kof2002d_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
}

/*************************************************
 kf2k1pa
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KF2K1PA_CART, neogeo_decrypt_kf2k1pa_cart_device, "neocart_kf2k1pa", "Neo Geo KoF 2001 Plus Cart")

neogeo_decrypt_kf2k1pa_cart_device::neogeo_decrypt_kf2k1pa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KF2K1PA_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kf2k1pa_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	uint32_t sx_size = fix_region_size;
	uint8_t *rom = fix_region;

	/* S-rom has its own unique encryption */
	for( int i = 0; i < sx_size; i++ )
		rom[ i ] = bitswap<8>( rom[ i ], 3, 2, 4, 5, 1, 6, 0, 7 );
}

/*************************************************
 kf2k2plsd
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KF2K2PLSD_CART, neogeo_decrypt_kf2k2plsd_cart_device, "neocart_kf2k2plsd", "Neo Geo KoF 2002 Plus Decrypted Cart")

neogeo_decrypt_kf2k2plsd_cart_device::neogeo_decrypt_kf2k2plsd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KF2K2PLSD_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kf2k2plsd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->kof2002_decrypt_68k(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
}

/*************************************************
 kf2k2mpd
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KF2K2MPD_CART, neogeo_decrypt_kf2k2mpd_cart_device, "neocart_kf2k2mpd", "Neo Geo KoF 2002 MP Decrypted Cart")

neogeo_decrypt_kf2k2mpd_cart_device::neogeo_decrypt_kf2k2mpd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KF2K2MPD_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kf2k2mpd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf2k2mp_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 2);
}

/*************************************************
 kf2k2mp2d
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KF2K2MP2D_CART, neogeo_decrypt_kf2k2mp2d_cart_device, "neocart_kf2k2mp2d", "Neo Geo KoF 2002 MP2 Decrypted Cart")

neogeo_decrypt_kf2k2mp2d_cart_device::neogeo_decrypt_kf2k2mp2d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KF2K2MP2D_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kf2k2mp2d_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_prot->kf2k2mp2_px_decrypt(cpuregion, cpuregion_size);
	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}

/*************************************************
 kof2k4pls
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF2K4PLS_CART, neogeo_decrypt_kof2k4pls_cart_device, "neocart_kof2k4pls", "Neo Geo KoF 2004 Plus bootleg Cart")

neogeo_decrypt_kof2k4pls_cart_device::neogeo_decrypt_kof2k4pls_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KOF2K4PLS_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kof2k4pls_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	uint8_t *src = cpuregion;
	std::vector<uint8_t> dst(0x200000);

	memcpy(&dst[0], src,0x100000);
	memcpy(&dst[0x100000], src + 0x500000, 0x100000);
	memcpy(src + 0x000000, &dst[0x1A0000], 0x020000);
	memcpy(src + 0x020000, &dst[0x080000], 0x020000);
	memcpy(src + 0x040000, &dst[0x140000], 0x020000);
	memcpy(src + 0x060000, &dst[0x000000], 0x020000);
	memcpy(src + 0x080000, &dst[0x180000], 0x020000);
	memcpy(src + 0x0A0000, &dst[0x0A0000], 0x020000);
	memcpy(src + 0x0C0000, &dst[0x100000], 0x020000);
	memcpy(src + 0x0E0000, &dst[0x040000], 0x020000);

	m_prot->sx_decrypt(fix_region, fix_region_size, 1);
}

/*************************************************
 kof10thu
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF10THU_CART, neogeo_decrypt_kof10thu_cart_device, "neocart_kof10thu", "Neo Geo KoF 10th Anniversary 2005 Unique bootleg set 2 Cart")

neogeo_decrypt_kof10thu_cart_device::neogeo_decrypt_kof10thu_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KOF10THU_CART, tag, owner, clock)
{
}

void neogeo_decrypt_kof10thu_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	uint32_t tsize = cpuregion_size;
	uint8_t *rom = cpuregion;
	uint32_t i, ofst;
	std::vector<uint8_t> trom( tsize );
	memcpy( &trom[ 0 ], &rom[ 0 ], tsize );
	// We unscramble the lower 6 address lines of the program rom.
	for( i = 0; i < tsize / 2; i++ )
	{
		ofst = bitswap<8>( (i & 0x0000ff), 7, 6, 2, 3, 4, 5, 0, 1 ) | (i & 0xffff00);
		memcpy( &rom[ ofst * 2 ], &trom[ i * 2 ], 2 );
	}
}

/*************************************************
 kogd
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_KOGD_CART, neogeo_decrypt_kogd_cart_device, "neocart_kogd", "Neo Geo King of Gladiators Bootleg Decrypted Cart")

neogeo_decrypt_kogd_cart_device::neogeo_decrypt_kogd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_KOGD_CART, tag, owner, clock),
	m_jumper(*this, "JUMPER")
{
}

static INPUT_PORTS_START( kogd )
	// a jumper on the pcb overlays a ROM address, very strange but that's how it works.
	PORT_START("JUMPER")
	PORT_DIPNAME( 0x0001, 0x0001, "Title Language" ) PORT_DIPLOCATION("CART-JUMPER:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, "Non-English" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor neogeo_decrypt_kogd_cart_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kogd );
}

uint16_t neogeo_decrypt_kogd_cart_device::protection_r(address_space &space, offs_t offset)
{
	return m_jumper->read();
}

/*************************************************
 matrimd
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_MATRIMD_CART, neogeo_decrypt_matrimd_cart_device, "neocart_matrimd", "Neo Geo Matrimelee Decrypted Cart")

neogeo_decrypt_matrimd_cart_device::neogeo_decrypt_matrimd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_MATRIMD_CART, tag, owner, clock)
{
}

void neogeo_decrypt_matrimd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->matrim_decrypt_68k(cpuregion, cpuregion_size);
}

/*************************************************
 samsho5d
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_SAMSHO5D_CART, neogeo_decrypt_samsho5d_cart_device, "neocart_samsho5d", "Neo Geo Samurai Shodown 5 Decrypted Cart")

neogeo_decrypt_samsho5d_cart_device::neogeo_decrypt_samsho5d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_SAMSHO5D_CART, tag, owner, clock)
{
}

void neogeo_decrypt_samsho5d_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->samsho5_decrypt_68k(cpuregion, cpuregion_size);
}

/*************************************************
 samsh5spd
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_SAMSHO5SPD_CART, neogeo_decrypt_samsho5spd_cart_device, "neocart_samsh5spd", "Neo Geo Samurai Shodown 5 Special Decrypted Cart")

neogeo_decrypt_samsho5spd_cart_device::neogeo_decrypt_samsho5spd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_SAMSHO5SPD_CART, tag, owner, clock)
{
}

void neogeo_decrypt_samsho5spd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_kof2k2_prot->samsh5sp_decrypt_68k(cpuregion, cpuregion_size);
}

// ================================================================> NEOGEO_DECRYPT_PVC_CART_DEVICE
/*************************************************
 mslug5d
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_MSLUG5D_CART, neogeo_decrypt_pvc_mslug5d_cart_device, "neocart_mslug5d", "Neo Geo Metal Slug 5 Decrypted PVC Cart")

neogeo_decrypt_pvc_mslug5d_cart_device::neogeo_decrypt_pvc_mslug5d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_pvc_cart_device(mconfig, NEOGEO_DECRYPT_PVC_MSLUG5D_CART, tag, owner, clock)
{
}

void neogeo_decrypt_pvc_mslug5d_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->mslug5_decrypt_68k(cpuregion, cpuregion_size);
}

/*************************************************
 mslug5e
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_MSLUG5E_CART, neogeo_decrypt_pvc_mslug5e_cart_device, "neocart_mslug5e", "Neo Geo Metal Slug 5 Protection PVC Cart")

neogeo_decrypt_pvc_mslug5e_cart_device::neogeo_decrypt_pvc_mslug5e_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_pvc_cart_device(mconfig, NEOGEO_DECRYPT_PVC_MSLUG5E_CART, tag, owner, clock)
{
}

/*************************************************
 kof2003d
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_KOF2003D_CART, neogeo_decrypt_pvc_kof2003d_cart_device, "neocart_kof2003d", "Neo Geo KoF 2003 Decrypted PVC Cart")

neogeo_decrypt_pvc_kof2003d_cart_device::neogeo_decrypt_pvc_kof2003d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_pvc_cart_device(mconfig, NEOGEO_DECRYPT_PVC_KOF2003D_CART, tag, owner, clock)
{
}

void neogeo_decrypt_pvc_kof2003d_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->kof2003_decrypt_68k(cpuregion, cpuregion_size);
}

/*************************************************
 kof2003hd
 **************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_KOF2003HD_CART, neogeo_decrypt_pvc_kof2003hd_cart_device, "neocart_kof2003hd", "Neo Geo KoF 2003 Decrypted AES PVC Cart")

neogeo_decrypt_pvc_kof2003hd_cart_device::neogeo_decrypt_pvc_kof2003hd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_pvc_cart_device(mconfig, NEOGEO_DECRYPT_PVC_KOF2003HD_CART, tag, owner, clock)
{
}

void neogeo_decrypt_pvc_kof2003hd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->kof2003h_decrypt_68k(cpuregion, cpuregion_size);
}

/*************************************************
 svcd
**************************************************/

DEFINE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_SVCD_CART, neogeo_decrypt_pvc_svcd_cart_device, "neocart_svcd", "Neo Geo SNK vs Capcom Decrypted PVC Cart")

neogeo_decrypt_pvc_svcd_cart_device::neogeo_decrypt_pvc_svcd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_decrypt_pvc_cart_device(mconfig, NEOGEO_DECRYPT_PVC_SVCD_CART, tag, owner, clock)
{
}

void neogeo_decrypt_pvc_svcd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_pvc_prot->svc_px_decrypt(cpuregion, cpuregion_size);
}

// ================================================================> NEOGEO_SMA_DECRYPT_CART_DEVICE
/*************************************************
 garoud
**************************************************/

neogeo_decrypt_sma_garoud_cart_device::neogeo_decrypt_sma_garoud_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_SMA_GAROUD_CART, tag, owner, clock)
{
}

void neogeo_decrypt_sma_garoud_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->garou_decrypt_68k(cpuregion);
}

/*************************************************
 garouhd
 **************************************************/

neogeo_decrypt_sma_garouhd_cart_device::neogeo_decrypt_sma_garouhd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_SMA_GAROUHD_CART, tag, owner, clock)
{
}

void neogeo_decrypt_sma_garouhd_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->garouh_decrypt_68k(cpuregion);
}

/*************************************************
 kof99d
**************************************************/

neogeo_decrypt_sma_kof99d_cart_device::neogeo_decrypt_sma_kof99d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_SMA_KOF99D_CART, tag, owner, clock)
{
}

void neogeo_decrypt_sma_kof99d_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->kof99_decrypt_68k(cpuregion);
}

/*************************************************
 kof2000d
**************************************************/

neogeo_decrypt_sma_kof2000d_cart_device::neogeo_decrypt_sma_kof2000d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_SMA_KOF2000D_CART, tag, owner, clock)
{
}

void neogeo_decrypt_sma_kof2000d_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->kof2000_decrypt_68k(cpuregion);
}

/*************************************************
 kof2000nd
**************************************************/

neogeo_decrypt_sma_kof2000nd_cart_device::neogeo_decrypt_sma_kof2000nd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_SMA_KOF2000ND_CART, tag, owner, clock)
{
}

/*************************************************
 mslug3d
 **************************************************/

neogeo_decrypt_sma_mslug3d_cart_device::neogeo_decrypt_sma_mslug3d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	neogeo_sma_decrypt_cart_device(mconfig, NEOGEO_DECRYPT_SMA_MSLUG3D_CART, tag, owner, clock)
{
}

void neogeo_decrypt_sma_mslug3d_cart_device::decrypt_all(DECRYPT_ALL_PARAMS)
{
	m_sma_prot->mslug3_decrypt_68k(cpuregion);
}
