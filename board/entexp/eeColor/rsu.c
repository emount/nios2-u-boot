#include "rsu.h"

//
// this call is used in factory/boot mode or application mode
//
uint32_t rsu_get_current_state_mode( uint32_t rsu_base ) {

	return( ( RD_RSU_STATE_MODE_REG( rsu_base ) & RSU_STATE_MODE_RD_MASK ) >> RSU_STATE_MODE_RD_OFST );
}

//
// this call is used in factory/boot mode only
//
void rsu_factory_get_previous_reconfig_status(
			uint32_t rsu_base,
			uint32_t* condition_1,
			uint32_t* condition_2,
			uint32_t* boot_address_1,
			uint32_t* boot_address_2
		) {

	*condition_1 = ( RD_RSU_PS1_RECONFIG_TRIGGER_CONDITIONS_REG( rsu_base ) & RSU_RECONFIG_TRIGGER_CONDITIONS_RD_MASK ) >> RSU_RECONFIG_TRIGGER_CONDITIONS_RD_OFST;
	*condition_2 = ( RD_RSU_PS2_RECONFIG_TRIGGER_CONDITIONS_REG( rsu_base ) & RSU_RECONFIG_TRIGGER_CONDITIONS_RD_MASK ) >> RSU_RECONFIG_TRIGGER_CONDITIONS_RD_OFST;
	*boot_address_1 = ( RD_RSU_PS1_BOOT_ADDRESS_REG( rsu_base ) & RSU_BOOT_ADDRESS_RD_MASK ) >> RSU_BOOT_ADDRESS_RD_OFST;
	*boot_address_2 = ( RD_RSU_PS2_BOOT_ADDRESS_REG( rsu_base ) & RSU_BOOT_ADDRESS_RD_MASK ) >> RSU_BOOT_ADDRESS_RD_OFST;

	return;
}

//
// this call is used in factory/boot mode only
//
void rsu_factory_trigger_reconfig(
			uint32_t rsu_base,
			uint32_t boot_address_value,		// MSB 22-bits of 24-bit address
			uint32_t watch_dog_value			// MSB 12-bits of 29-bit timeout count
		) {

	WR_RSU_FORCE_EARLY_CONF_DONE_CHECK_REG( rsu_base, ( 0x01 << RSU_FORCE_EARLY_CONF_DONE_CHECK_WR_OFST ) & RSU_FORCE_EARLY_CONF_DONE_CHECK_WR_MASK );

	WR_RSU_WATCHDOG_TIMEOUT_VALUE_REG( rsu_base, ( watch_dog_value << RSU_WATCHDOG_TIMEOUT_VALUE_WR_OFST ) & RSU_WATCHDOG_TIMEOUT_VALUE_WR_MASK );

	if( watch_dog_value > 0 ) {

		WR_RSU_WATCHDOG_ENABLE_REG( rsu_base, ( 0x01 << RSU_WATCHDOG_ENABLE_WR_OFST ) & RSU_WATCHDOG_ENABLE_WR_MASK );

	} else {

		WR_RSU_WATCHDOG_ENABLE_REG( rsu_base, ( 0x00 << RSU_WATCHDOG_ENABLE_WR_OFST ) & RSU_WATCHDOG_ENABLE_WR_MASK );
	}

	WR_RSU_BOOT_ADDRESS_REG( rsu_base, ( boot_address_value << RSU_BOOT_ADDRESS_WR_OFST ) & RSU_BOOT_ADDRESS_WR_MASK );

	WR_RSU_FORCE_INTERNAL_OSC_REG( rsu_base, ( 0x01 << RSU_FORCE_INTERNAL_OSC_WR_OFST ) & RSU_FORCE_INTERNAL_OSC_WR_MASK );

	WR_RSU_CONTROL_STATUS_REG( rsu_base, ( RSU_CONTROL_STATUS_RECONFIG << RSU_CONTROL_STATUS_OFST ) & RSU_CONTROL_STATUS_MASK );

	while(1);	// at this point we should undergo hardware reconfiguration

	return;
}

//
// this call is used in application mode only
//
void rsu_app_get_current_wd(
			uint32_t rsu_base,
			uint32_t* current_wd_count,
			uint32_t* reset_timer_state
		) {

	*current_wd_count = ( RD_RSU_BOOT_ADDRESS_REG( rsu_base ) & RSU_WATCHDOG_TIMEOUT_VALUE_RD_MASK ) >> RSU_WATCHDOG_TIMEOUT_VALUE_RD_OFST;

	*reset_timer_state = ((( RD_RSU_CONTROL_STATUS_REG( rsu_base ) & RSU_CONTROL_STATUS_MASK ) >> RSU_CONTROL_STATUS_OFST ) & RSU_CONTROL_STATUS_RESET_TIMER ) ? ( 1 ) : ( 0 );

	return;
}

//
// this call is used in application mode only
//
uint32_t rsu_app_set_reset_timer_state(
			uint32_t rsu_base,
			uint32_t new_state
		) {
	uint32_t temp;

	temp = ( RD_RSU_CONTROL_STATUS_REG( rsu_base ) & RSU_CONTROL_STATUS_MASK ) >> RSU_CONTROL_STATUS_OFST;

	if( new_state > 0 ) {

		temp = temp | RSU_CONTROL_STATUS_RESET_TIMER;
	} else {

		temp = temp & ~RSU_CONTROL_STATUS_RESET_TIMER;
	}

	WR_RSU_CONTROL_STATUS_REG( rsu_base, ( temp << RSU_CONTROL_STATUS_OFST ) & RSU_CONTROL_STATUS_MASK );

	return(((( RD_RSU_CONTROL_STATUS_REG( rsu_base ) & RSU_CONTROL_STATUS_MASK ) >> RSU_CONTROL_STATUS_OFST ) & RSU_CONTROL_STATUS_RESET_TIMER ) ? ( 1 ) : ( 0 ));
}

//
// this call is used in application mode only
//
void rsu_app_trigger_reconfig( uint32_t rsu_base ) {

	uint32_t temp;

	temp = ( RD_RSU_CONTROL_STATUS_REG( rsu_base ) & RSU_CONTROL_STATUS_MASK ) >> RSU_CONTROL_STATUS_OFST;

	temp = temp | RSU_CONTROL_STATUS_RECONFIG;

	WR_RSU_CONTROL_STATUS_REG( rsu_base, ( temp << RSU_CONTROL_STATUS_OFST ) & RSU_CONTROL_STATUS_MASK );

	while(1);	// at this point we should undergo hardware reconfiguration

	return;
}
