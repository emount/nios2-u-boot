#ifndef RSU_H_
#define RSU_H_

#include <common.h>
#include "io.h"

uint32_t rsu_get_current_state_mode( uint32_t rsu_base );

void rsu_factory_get_previous_reconfig_status(
			uint32_t rsu_base,
			uint32_t* condition_1,
			uint32_t* condition_2,
			uint32_t* boot_address_1,
			uint32_t* boot_address_2
		);
void rsu_factory_trigger_reconfig(
			uint32_t rsu_base,
			uint32_t boot_address_value,		// MSB 22-bits of 24-bit address
			uint32_t watch_dog_value			// MSB 12-bits of 29-bit timeout count
		);
void rsu_app_get_current_wd(
			uint32_t rsu_base,
			uint32_t* current_wd_count,
			uint32_t* reset_timer_state
		);
uint32_t rsu_app_set_reset_timer_state(
			uint32_t rsu_base,
			uint32_t new_state
		);
void rsu_app_trigger_reconfig( uint32_t rsu_base );

//
//	Macro definitions for Remote Update Controller Cyclone IV
//

//
//	Master State Machine Current State Mode
//	This read only field is valid in factory and application run times.
//
// access macro
#define RD_RSU_STATE_MODE_REG( base ) \
	IORD_32DIRECT( base, ( 0x00 << 2 ) )
// offset and mask for bit field
#define RSU_STATE_MODE_RD_OFST	( 0x01 )
#define RSU_STATE_MODE_RD_MASK	( 0x06 )
// bit field encodings
#define RSU_STATE_MODE_FACTORY	( 0x00 )
#define RSU_STATE_MODE_APP		( 0x01 )
#define RSU_STATE_MODE_APP_WD	( 0x03 )

//
//	Force early CONF_DONE (Cd_early) check
//	This write only field is valid only in factory run time.
//
// access macro
#define WR_RSU_FORCE_EARLY_CONF_DONE_CHECK_REG( base, data ) \
	IOWR_32DIRECT( base, ( 0x01 << 2 ), data )
// offset and mask for bit field
#define RSU_FORCE_EARLY_CONF_DONE_CHECK_WR_OFST	( 0x00 )
#define RSU_FORCE_EARLY_CONF_DONE_CHECK_WR_MASK	( 0x01 )

//
//	Watchdog Timeout Value
//	This write only field is valid only in factory run time.
//
// access macro
#define WR_RSU_WATCHDOG_TIMEOUT_VALUE_REG( base, data ) \
	IOWR_32DIRECT( base, ( 0x02 << 2 ), data )
// offset and mask for bit field
#define RSU_WATCHDOG_TIMEOUT_VALUE_WR_OFST	( 0x00 )
#define RSU_WATCHDOG_TIMEOUT_VALUE_WR_MASK	( 0xFFF )

//
//	Watchdog Enable
//	This write only field is valid only in factory run time.
//
// access macro
#define WR_RSU_WATCHDOG_ENABLE_REG( base, data ) \
	IOWR_32DIRECT( base, ( 0x03 << 2 ), data )
// offset and mask for bit field
#define RSU_WATCHDOG_ENABLE_WR_OFST ( 0x00 )
#define RSU_WATCHDOG_ENABLE_WR_MASK ( 0x01 )

//
//	Boot Address
//	This read/write field is valid in factory and application run times, however
//	the meaning of the field changes in each.
//
//	During the factory run time, this field can be written with a new address
//	value that points to the next desired reconfiguration location in the flash.
//	And it can be read from the PS1 and PS2 offsets to view the base address
//	location that was used in the previous 1 and 2 reconfigurations ago.  Reading
//	the current state of this field has no meaning.
//
//	During the application run time, this field can be read from the current
//	status offset to view the state of the watch dog timer.  But writing this
//	field is meaningless.
//
// access macro - for application read of current watch dog timer value
#define RD_RSU_BOOT_ADDRESS_REG( base ) \
	IORD_32DIRECT( base, ( 0x04 << 2 ) )
// access macro - for factory read of last reconfiguration base address value
#define RD_RSU_PS1_BOOT_ADDRESS_REG( base ) \
	IORD_32DIRECT( base, ( 0x0C << 2 ) )
// access macro - for factory read of 2 reconfigurations ago, base address value
#define RD_RSU_PS2_BOOT_ADDRESS_REG( base ) \
	IORD_32DIRECT( base, ( 0x14 << 2 ) )
// access macro - for factory write of next base address value
#define WR_RSU_BOOT_ADDRESS_REG( base, data ) \
	IOWR_32DIRECT( base, ( 0x04 << 2 ), data )
// offset and mask for bit field during factory write
#define RSU_BOOT_ADDRESS_WR_OFST	( 0x00 )
#define RSU_BOOT_ADDRESS_WR_MASK	( 0x003FFFFF )
// offset and mask for bit field during factory read
#define RSU_BOOT_ADDRESS_RD_OFST	( 0x01 )
#define RSU_BOOT_ADDRESS_RD_MASK	( 0x01FFFFFE )
// offset and mask for bit field during application read
// this register does not appear to read back properly, the value is << 1 bit to
// much and the upper 5 bits of the counter always report back ZEROs, not going
// to adjust it at all
#define RSU_WATCHDOG_TIMEOUT_VALUE_RD_OFST	( 0x00 )
#define RSU_WATCHDOG_TIMEOUT_VALUE_RD_MASK	( 0xFFFFFFFF )

//
//	illegal value
//	This register offset is always an illegal access.
//
//#define RD_RSU_ILLEGAL_REG( base )
//	IORD_32DIRECT( base, ( 0x05 << 2 ) )
//#define WR_RSU_ILLEGAL_REG( base, data )
//	IOWR_32DIRECT( base, ( 0x05 << 2 ), data )

//
//	Force the internal oscillator as startup state machine clock
//	This write only field is valid only in factory run time.
//
// access macro
#define WR_RSU_FORCE_INTERNAL_OSC_REG( base, data ) \
	IOWR_32DIRECT( base, ( 0x06 << 2 ), data )
// offset and mask for bit field
#define RSU_FORCE_INTERNAL_OSC_WR_OFST	( 0x00 )
#define RSU_FORCE_INTERNAL_OSC_WR_MASK	( 0x01 )

//
//	Reconfiguration trigger conditions
//	This read only field is valid only in factory run time.
//
//	During the factory run time, this field can be read from the PS1 and PS2
//	offsets to view the reconfig trigger conditions that occurred in the previous
//	1 and 2 reconfigurations ago.  Reading the current state of this field has no
//	meaning.
//
// access macro - for factory read of last reconfiguration condition value
#define RD_RSU_PS1_RECONFIG_TRIGGER_CONDITIONS_REG( base ) \
	IORD_32DIRECT( base, ( 0x0F << 2 ) )
// access macro - for factory read of 2 reconfigurations ago, reconfiguration condition value
#define RD_RSU_PS2_RECONFIG_TRIGGER_CONDITIONS_REG( base ) \
	IORD_32DIRECT( base, ( 0x17 << 2 ) )
// offset and mask for bit field
#define RSU_RECONFIG_TRIGGER_CONDITIONS_RD_OFST	( 0x01 )
#define RSU_RECONFIG_TRIGGER_CONDITIONS_RD_MASK	( 0x3E )
// bit field encodings
#define RSU_RECONFIG_TRIGGER_CONDITIONS_RUNCONFIG	( 0x01 )
#define RSU_RECONFIG_TRIGGER_CONDITIONS_WDTIMER		( 0x02 )
#define RSU_RECONFIG_TRIGGER_CONDITIONS_NSTATUS		( 0x04 )
#define RSU_RECONFIG_TRIGGER_CONDITIONS_CRCERROR	( 0x08 )
#define RSU_RECONFIG_TRIGGER_CONDITIONS_NCONFIG		( 0x10 )

//
//	Control and Status
//	This read/write field is valid in factory and application run times.
//
// access macros
#define RD_RSU_CONTROL_STATUS_REG( base ) \
	IORD_32DIRECT( base, ( 0x20 << 2 ) )
#define WR_RSU_CONTROL_STATUS_REG( base, data ) \
	IOWR_32DIRECT( base, ( 0x20 << 2 ), data )
// offset and mask for bit field
#define RSU_CONTROL_STATUS_OFST	( 0x00 )
#define RSU_CONTROL_STATUS_MASK	( 0x03 )
// bit field encodings
#define RSU_CONTROL_STATUS_RECONFIG		( 0x01 )
#define RSU_CONTROL_STATUS_RESET_TIMER	( 0x02 )

#endif /* RSU_H_ */
