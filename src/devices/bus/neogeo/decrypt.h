// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli,Gaston90
#ifndef MAME_BUS_NEOGEO_DECRYPT_H
#define MAME_BUS_NEOGEO_DECRYPT_H

#pragma once

#include "slot.h"
#include "rom.h"
#include "prot_misc.h"
#include "prot_pcm2.h"
#include "prot_cmc.h"
#include "prot_cthd.h"
#include "prot_pvc.h"
#include "prot_sma.h"
#include "prot_kof98.h"
#include "prot_kof2k2.h"
#include "prot_kof2k3bl.h"
#include "machine/nvram.h"

// ================================================================> NEOGEO_DECRYPT_CART_DEVICE

class neogeo_decrypt_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_decrypt_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override { }
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_decrypt_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2002_prot_device> m_kof2k2_prot;
	required_device<neoboot_prot_device> m_prot;
    required_device<cthd_prot_device> m_cthd_prot;
    required_device<kof98_prot_device> m_kof98_prot;
    required_device<kof2k3bl_prot_device> m_kof2k3bl_prot;
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_CART, neogeo_decrypt_cart_device)

// ======================> neogeo_decrypt_pvc_cart_device

class neogeo_decrypt_pvc_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_decrypt_pvc_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_pvc_prot->get_bank_base(); }
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_pvc_prot->protection_r(offset); }
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { m_pvc_prot->protection_w(offset, data, mem_mask); }

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override { }
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_decrypt_pvc_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<neoboot_prot_device> m_prot;
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<pvc_prot_device> m_pvc_prot;
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_CART, neogeo_decrypt_pvc_cart_device)

// ================================================================> NEOGEO_SMA_DECRYPT_CART_DEVICE

class neogeo_sma_decrypt_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_sma_decrypt_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual uint16_t protection_r(address_space &space, offs_t offset) override { return m_sma_prot->prot_9a37_r(); }
	virtual uint16_t addon_r(offs_t offset) override { return m_sma_prot->random_r(); }
	virtual uint32_t get_bank_base(uint16_t sel) override { return 0; }

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_sma_decrypt_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<sma_prot_device> m_sma_prot;
	required_device<cmc_prot_device> m_cmc_prot;

};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_SMA_DECRYPT_CART, neogeo_sma_decrypt_cart_device)

// ================================================================> NEOGEO_DARKSOFT_CTHD2K3_CART_DEVICE
/*************************************************
 cthd2k3a
 **************************************************/

class neogeo_decrypt_cthd2k3a_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_cthd2k3a_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_CTHD2K3A_CART, neogeo_decrypt_cthd2k3a_cart_device)

// ================================================================> NEOGEO_DECRYPT_CART_DEVICE

/*************************************************
 jckeygpd
 **************************************************/

class neogeo_decrypt_jockeygpd_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_jockeygpd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int get_fixed_bank_type() override { return 1; }

	virtual uint16_t ram_r(offs_t offset) override { return m_ram[offset]; }
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { COMBINE_DATA(&m_ram[offset]); }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	std::unique_ptr<uint16_t[]> m_ram;

	required_device<nvram_device> m_nvram;
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_JOCKEYGPD_CART, neogeo_decrypt_jockeygpd_cart_device)

/*************************************************
 kof96ep
 **************************************************/

class neogeo_decrypt_kof96ep_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kof96ep_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF96EP_CART, neogeo_decrypt_kof96ep_cart_device)

/*************************************************
 kof97pla
 **************************************************/

class neogeo_decrypt_kof97pla_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kof97pla_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF97PLA_CART, neogeo_decrypt_kof97pla_cart_device)

/*************************************************
 kof2002d
**************************************************/

class neogeo_decrypt_kof2002d_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kof2002d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF2002D_CART, neogeo_decrypt_kof2002d_cart_device)

/*************************************************
 kf2k1pa
**************************************************/

class neogeo_decrypt_kf2k1pa_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kf2k1pa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KF2K1PA_CART, neogeo_decrypt_kf2k1pa_cart_device)

/*************************************************
 kf2k2plsd
 **************************************************/

class neogeo_decrypt_kf2k2plsd_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kf2k2plsd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KF2K2PLSD_CART, neogeo_decrypt_kf2k2plsd_cart_device)

/*************************************************
 kf2k2mpd
 **************************************************/

class neogeo_decrypt_kf2k2mpd_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kf2k2mpd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KF2K2MPD_CART, neogeo_decrypt_kf2k2mpd_cart_device)

/*************************************************
 kf2k2mp2d
 **************************************************/

class neogeo_decrypt_kf2k2mp2d_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kf2k2mp2d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KF2K2MP2D_CART, neogeo_decrypt_kf2k2mp2d_cart_device)

/*************************************************
 kof2k4pls
 **************************************************/

class neogeo_decrypt_kof2k4pls_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kof2k4pls_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF2K4PLS_CART, neogeo_decrypt_kof2k4pls_cart_device)

/*************************************************
 kof10thu
 **************************************************/

class neogeo_decrypt_kof10thu_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kof10thu_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KOF10THU_CART, neogeo_decrypt_kof10thu_cart_device)

/*************************************************
 kogd
**************************************************/

class neogeo_decrypt_kogd_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_kogd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint16_t protection_r(address_space &space, offs_t offset) override;
	virtual int get_fixed_bank_type() override { return 0; }

private:
	required_ioport m_jumper;
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_KOGD_CART, neogeo_decrypt_kogd_cart_device)

/*************************************************
 matrimd
**************************************************/

class neogeo_decrypt_matrimd_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_matrimd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_MATRIMD_CART, neogeo_decrypt_matrimd_cart_device)

/*************************************************
 samsho5d
**************************************************/

class neogeo_decrypt_samsho5d_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_samsho5d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_SAMSHO5D_CART, neogeo_decrypt_samsho5d_cart_device)

/*************************************************
 samsho5spd
**************************************************/

class neogeo_decrypt_samsho5spd_cart_device : public neogeo_decrypt_cart_device
{
public:
	neogeo_decrypt_samsho5spd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_SAMSHO5SPD_CART, neogeo_decrypt_samsho5spd_cart_device)

// ================================================================> NEOGEO_DECRYPT_PVC_CART_DEVICE
/*************************************************
 mslug5d
**************************************************/

class neogeo_decrypt_pvc_mslug5d_cart_device : public neogeo_decrypt_pvc_cart_device
{
public:
	neogeo_decrypt_pvc_mslug5d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_MSLUG5D_CART, neogeo_decrypt_pvc_mslug5d_cart_device)

/*************************************************
 mslug5e
**************************************************/

class neogeo_decrypt_pvc_mslug5e_cart_device : public neogeo_decrypt_pvc_cart_device
{
public:
	neogeo_decrypt_pvc_mslug5e_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_MSLUG5E_CART, neogeo_decrypt_pvc_mslug5e_cart_device)

/*************************************************
 kof2003d
**************************************************/

class neogeo_decrypt_pvc_kof2003d_cart_device : public neogeo_decrypt_pvc_cart_device
{
public:
	neogeo_decrypt_pvc_kof2003d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_KOF2003D_CART, neogeo_decrypt_pvc_kof2003d_cart_device)

/*************************************************
 kof2003hd
**************************************************/

class neogeo_decrypt_pvc_kof2003hd_cart_device : public neogeo_decrypt_pvc_cart_device
{
public:
	neogeo_decrypt_pvc_kof2003hd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_KOF2003HD_CART, neogeo_decrypt_pvc_kof2003hd_cart_device)

/*************************************************
 svcd
**************************************************/

class neogeo_decrypt_pvc_svcd_cart_device : public neogeo_decrypt_pvc_cart_device
{
public:
	neogeo_decrypt_pvc_svcd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_PVC_SVCD_CART, neogeo_decrypt_pvc_svcd_cart_device)

// ================================================================> NEOGEO_SMA_DECRYPT_CART_DEVICE
/*************************************************
 garoud
**************************************************/

class neogeo_decrypt_sma_garoud_cart_device : public neogeo_sma_decrypt_cart_device
{
public:
	neogeo_decrypt_sma_garoud_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->garou_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_GAROUD_CART, neogeo_decrypt_sma_garoud_cart_device)

/*************************************************
 garouhd
 **************************************************/

class neogeo_decrypt_sma_garouhd_cart_device : public neogeo_sma_decrypt_cart_device
{
public:
	neogeo_decrypt_sma_garouhd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->garouh_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_GAROUHD_CART, neogeo_decrypt_sma_garouhd_cart_device)

/*************************************************
 kof99d
**************************************************/

class neogeo_decrypt_sma_kof99d_cart_device : public neogeo_sma_decrypt_cart_device
{
public:
	neogeo_decrypt_sma_kof99d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->kof99_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_KOF99D_CART, neogeo_decrypt_sma_kof99d_cart_device)

/*************************************************
 kof2000d
**************************************************/

class neogeo_decrypt_sma_kof2000d_cart_device : public neogeo_sma_decrypt_cart_device
{
public:
	neogeo_decrypt_sma_kof2000d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 2; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->kof2000_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_KOF2000D_CART, neogeo_decrypt_sma_kof2000d_cart_device)

/*************************************************
 kof2000nd
**************************************************/

class neogeo_decrypt_sma_kof2000nd_cart_device : public neogeo_sma_decrypt_cart_device
{
public:
	neogeo_decrypt_sma_kof2000nd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int get_fixed_bank_type() override { return 2; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->kof2000_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_KOF2000ND_CART, neogeo_decrypt_sma_kof2000nd_cart_device)

/*************************************************
 mslug3d
 **************************************************/

class neogeo_decrypt_sma_mslug3d_cart_device : public neogeo_sma_decrypt_cart_device
{
public:
	neogeo_decrypt_sma_mslug3d_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
	virtual uint32_t get_bank_base(uint16_t sel) override { return m_sma_prot->mslug3_bank_base(sel); }
};

DECLARE_DEVICE_TYPE(NEOGEO_DECRYPT_SMA_MSLUG3D_CART, neogeo_decrypt_sma_mslug3d_cart_device)

#endif // MAME_BUS_NEOGEO_DECRYPT_H
