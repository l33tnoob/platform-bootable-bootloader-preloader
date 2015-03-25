#include <typedefs.h>
#include <platform.h>
#include <pmic_wrap_init.h>
#include <pmic.h>
#include <mt6311.h>

//==============================================================================
// PMIC access API
//==============================================================================
U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic_reg = 0;
    U32 rdata;    

    //mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;
    if(return_value!=0)
    {   
        print("[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= (MASK << SHIFT);
    *val = (pmic_reg >> SHIFT);    
    //print("[pmic_read_interface] val=0x%x\n", *val);

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic_reg = 0;
    U32 rdata;

    //1. mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
    
    pmic_reg &= ~(MASK << SHIFT);
    pmic_reg |= (val << SHIFT);

    //2. mt_write_byte(RegNum, pmic_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic_reg, &rdata);
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic_reg);    

#if 0
    //3. Double Check    
    //mt_read_byte(RegNum, &pmic_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic_reg=rdata;    
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    print("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic_reg);
#endif    

    return return_value;
}

void upmu_set_reg_value(kal_uint32 reg, kal_uint32 reg_val)
{
    U32 ret=0;
    
    ret=pmic_config_interface(reg, reg_val, 0xFFFF, 0x0);    
}


//==============================================================================
// PMIC-Charger Type Detection
//==============================================================================
CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE charger_tye;
    
    charger_tye = STANDARD_HOST;
    
    return charger_tye;
}

CHARGER_TYPE mt_charger_type_detection(void)
{
    if( g_first_check == 0 )
    {
        g_first_check = 1;
        g_ret = hw_charger_type_detection();
    }
    else
    {
        printf("[mt_charger_type_detection] Got data !!, %d, %d\r\n", g_charger_in_flag, g_first_check);
    }

    return g_ret;
}

//==============================================================================
// PMIC Usage APIs
//==============================================================================
U32 get_MT6328_PMIC_chip_version (void)
{
    U32 ret=0;
    U32 val=0;
    
    ret=pmic_read_interface( (U32)(MT6328_SWCID),
                           (&val),
                           (U32)(MT6328_PMIC_SWCID_MASK),
                           (U32)(MT6328_PMIC_SWCID_SHIFT)
	                       );	                       	                                                  

    return val;
}

int pmic_detect_powerkey(void)
{
    U32 ret=0;
    U32 val=0;
    
    ret=pmic_read_interface( (U32)(MT6328_TOPSTATUS),
                           (&val),
                           (U32)(MT6328_PMIC_PWRKEY_DEB_MASK),
                           (U32)(MT6328_PMIC_PWRKEY_DEB_SHIFT)
	                       );                             

    if (val==1){     
        printf("pl pmic powerkey Release\n");
        return 0;
    }else{
        printf("pl pmic powerkey Press\n");
        return 1;
    }
}

int pmic_detect_homekey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(MT6328_TOPSTATUS),
                           (&val),
                           (U32)(MT6328_PMIC_HOMEKEY_DEB_MASK),
                           (U32)(MT6328_PMIC_HOMEKEY_DEB_SHIFT)
	                       );
	                                                    
    if (val==1){     
        printf("pl pmic FCHRKEY Release\n");
        return 0;
    }else{
        printf("pl pmic FCHRKEY Press\n");
        return 1;
    }
}

U32 pmic_IsUsbCableIn (void) 
{    
    U32 ret=0;
    U32 val=0;

#if CFG_EVB_PLATFORM
    val = 1; // for bring up
    //printf("[pmic_IsUsbCableIn] have CFG_EVB_PLATFORM, %d\n", val);
#else
    ret=pmic_read_interface( (U32)(MT6328_CHR_CON0),
                           (&val),
                           (U32)(MT6328_PMIC_RGS_CHRDET_MASK),
                           (U32)(MT6328_PMIC_RGS_CHRDET_SHIFT)
	                       );
    printf("[pmic_IsUsbCableIn] %d\n", val);
#endif    

    if(val)
        return PMIC_CHRDET_EXIST;
    else
        return PMIC_CHRDET_NOT_EXIST;
}    

static int vbat_status = PMIC_VBAT_NOT_DROP;
static void pmic_DetectVbatDrop (void) 
{    

	U32 ret=0;
	U32 just_rst=0;

	pmic_read_interface( MT6328_STRUP_CON9, (&just_rst), MT6328_PMIC_JUST_PWRKEY_RST_MASK, MT6328_PMIC_JUST_PWRKEY_RST_SHIFT );
	pmic_config_interface(MT6328_STRUP_CON9, 1, MT6328_PMIC_CLR_JUST_RST_MASK, MT6328_PMIC_CLR_JUST_RST_SHIFT);

	printf("just_rst = %d\n", just_rst);
	if(just_rst)
		vbat_status = PMIC_VBAT_DROP;
	else
		vbat_status = PMIC_VBAT_NOT_DROP;

}

int pmic_IsVbatDrop(void)
{
   return vbat_status;	
}


void hw_set_cc(int cc_val)
{
    //TBD
}

void mt6325_upmu_set_baton_tdet_en(U32 val)
{
    U32 ret=0;  
    ret=pmic_config_interface( (U32)(MT6328_CHR_CON7),
                             (U32)(val),
                             (U32)(MT6328_PMIC_BATON_TDET_EN_MASK),
                             (U32)(MT6328_PMIC_BATON_TDET_EN_SHIFT)
	                         );  
}

void mt6325_upmu_set_rg_baton_en(U32 val)
{
    U32 ret=0;
    ret=pmic_config_interface( (U32)(MT6328_CHR_CON7),
                             (U32)(val),
                             (U32)(MT6328_PMIC_RG_BATON_EN_MASK),
                             (U32)(MT6328_PMIC_RG_BATON_EN_SHIFT)
	                         );  
}

U32 mt6325_upmu_get_rgs_baton_undet(void)
{
    U32 ret=0;
    U32 val=0;  
    ret=pmic_read_interface( (U32)(MT6328_CHR_CON41),
                           (&val),
                           (U32)(MT6328_PMIC_RGS_BATON_UNDET_MASK),
                           (U32)(MT6328_PMIC_RGS_BATON_UNDET_SHIFT)
	                       );
   return val;
}

int hw_check_battery(void)
{
    #ifdef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
        printf("ignore bat check !\n");
        return 1;
    #else
        #if CFG_EVB_PLATFORM
            printf("ignore bat check\n");
            return 1;
        #else
            U32 val=0;
			U32 ret_val;

			ret_val=pmic_config_interface( (U32)(MT6328_VTREF_CON0),
									 (U32)(1),
									 (U32)(MT6328_PMIC_RG_TREF_EN_MASK),
									 (U32)(MT6328_PMIC_RG_TREF_EN_SHIFT)
									 );  

			ret_val=pmic_config_interface( (U32)(MT6328_VTREF_CON0),
									 (U32)(1),
									 (U32)(MT6328_PMIC_RG_TREF_ON_CTRL_MASK),
									 (U32)(MT6328_PMIC_RG_TREF_ON_CTRL_SHIFT)
									 );  



            mt6325_upmu_set_baton_tdet_en(1);    
            mt6325_upmu_set_rg_baton_en(1);
            val = mt6325_upmu_get_rgs_baton_undet();

            if(val==0)
            {
                printf("bat is exist\n");
                return 1;
            }
            else
            {
                printf("bat NOT exist\n");
                return 0;
            }
        #endif
    #endif
}

void PMIC_enable_long_press_reboot(void)
{
#if !CFG_FPGA_PLATFORM
#if !CFG_EVB_PLATFORM
#if KPD_PMIC_LPRST_TD!=0
	#if ONEKEY_REBOOT_NORMAL_MODE_PL
	pmic_config_interface(MT6328_TOP_RST_MISC, 0x01, MT6328_PMIC_RG_PWRKEY_RST_EN_MASK, MT6328_PMIC_RG_PWRKEY_RST_EN_SHIFT);
	pmic_config_interface(MT6328_TOP_RST_MISC, 0x00, MT6328_PMIC_RG_HOMEKEY_RST_EN_MASK, MT6328_PMIC_RG_HOMEKEY_RST_EN_SHIFT);
	pmic_config_interface(MT6328_TOP_RST_MISC, (U32)KPD_PMIC_LPRST_TD, MT6328_PMIC_RG_PWRKEY_RST_TD_MASK, MT6328_PMIC_RG_PWRKEY_RST_TD_SHIFT);
	#else
	pmic_config_interface(MT6328_TOP_RST_MISC, 0x01, MT6328_PMIC_RG_PWRKEY_RST_EN_MASK, MT6328_PMIC_RG_PWRKEY_RST_EN_SHIFT);
	pmic_config_interface(MT6328_TOP_RST_MISC, 0x01, MT6328_PMIC_RG_HOMEKEY_RST_EN_MASK, MT6328_PMIC_RG_HOMEKEY_RST_EN_SHIFT);
	pmic_config_interface(MT6328_TOP_RST_MISC, (U32)KPD_PMIC_LPRST_TD, MT6328_PMIC_RG_PWRKEY_RST_TD_MASK, MT6328_PMIC_RG_PWRKEY_RST_TD_SHIFT);
	#endif
#else
	pmic_config_interface(MT6328_TOP_RST_MISC, 0x00, MT6328_PMIC_RG_PWRKEY_RST_EN_MASK, MT6328_PMIC_RG_PWRKEY_RST_EN_SHIFT);
	pmic_config_interface(MT6328_TOP_RST_MISC, 0x00, MT6328_PMIC_RG_HOMEKEY_RST_EN_MASK, MT6328_PMIC_RG_HOMEKEY_RST_EN_SHIFT);
#endif
#endif
#else
	pmic_config_interface(MT6328_TOP_RST_MISC, 0x00, MT6328_PMIC_RG_PWRKEY_RST_EN_MASK, MT6328_PMIC_RG_PWRKEY_RST_EN_SHIFT);
	pmic_config_interface(MT6328_TOP_RST_MISC, 0x00, MT6328_PMIC_RG_HOMEKEY_RST_EN_MASK, MT6328_PMIC_RG_HOMEKEY_RST_EN_SHIFT);
#endif
}

U32 PMIC_VUSB_EN(void)
{
	int ret=0;

	ret = pmic_config_interface( (kal_uint32)(MT6328_VUSB33_CON0),
			(kal_uint32)(1),
			(kal_uint32)(MT6328_PMIC_RG_VUSB33_EN_MASK),
			(kal_uint32)(MT6328_PMIC_RG_VUSB33_EN_SHIFT)
			);

	return ret;
}


void pl_charging(int en_chr)
{
    //TBD
}

void pl_kick_chr_wdt(void)
{
    //TBD
}

void pl_close_pre_chr_led(void)
{
    //no charger feature
}

U32 upmu_get_reg_value(kal_uint32 reg)
{
    U32 ret=0;
    U32 reg_val=0;
    
    ret=pmic_read_interface(reg, &reg_val, 0xFFFF, 0x0);
    
    return reg_val;
}



kal_uint16 pmic_read_efuse(kal_uint16 addr)
{
	kal_uint32 i,j;
	kal_uint32 ret,reg_val;

	pmic_config_interface(MT6328_PMIC_RG_OTP_PA_ADDR, addr*2, MT6328_PMIC_RG_OTP_PA_MASK, MT6328_PMIC_RG_OTP_PA_SHIFT);
	ret=pmic_read_interface(MT6328_PMIC_RG_OTP_RD_TRIG_ADDR, &reg_val, MT6328_PMIC_RG_OTP_RD_TRIG_MASK, MT6328_PMIC_RG_OTP_RD_TRIG_SHIFT);

	if(reg_val==0)
	{
		pmic_config_interface(MT6328_PMIC_RG_OTP_RD_TRIG_ADDR, 1, MT6328_PMIC_RG_OTP_RD_TRIG_MASK, MT6328_PMIC_RG_OTP_RD_TRIG_SHIFT);
	}
	else
	{
		pmic_config_interface(MT6328_PMIC_RG_OTP_RD_TRIG_ADDR, 0, MT6328_PMIC_RG_OTP_RD_TRIG_MASK, MT6328_PMIC_RG_OTP_RD_TRIG_SHIFT);
	}

	do
	{
	ret=pmic_read_interface(MT6328_PMIC_RG_OTP_RD_BUSY_ADDR, &reg_val, MT6328_PMIC_RG_OTP_RD_BUSY_MASK, MT6328_PMIC_RG_OTP_RD_BUSY_SHIFT);	
	}while(reg_val==1);

	ret=pmic_read_interface(MT6328_PMIC_RG_OTP_DOUT_SW_ADDR, &reg_val, MT6328_PMIC_RG_OTP_DOUT_SW_MASK, MT6328_PMIC_RG_OTP_DOUT_SW_SHIFT);
	
	return reg_val;

}


void pmic_6328_efuse_check(void)
{
    print("[0x%x]=0x%x\n", 0x0434, upmu_get_reg_value(0x0434));
    print("[0x%x]=0x%x\n", 0x0438, upmu_get_reg_value(0x0438));
    print("[0x%x]=0x%x\n", 0x0464, upmu_get_reg_value(0x0464));


    print("[0x%x]=0x%x\n", 0x0438, upmu_get_reg_value(0x0438));
    print("[0x%x]=0x%x\n", 0x046e, upmu_get_reg_value(0x046e));
	
    print("[0x%x]=0x%x\n", 0x0444, upmu_get_reg_value(0x0444));	
    print("[0x%x]=0x%x\n", 0x0458, upmu_get_reg_value(0x0458));	
    print("[0x%x]=0x%x\n", 0x044e, upmu_get_reg_value(0x044e));		


    print("[0x%x]=0x%x\n", 0x0a52, upmu_get_reg_value(0x0a52));
    print("[0x%x]=0x%x\n", 0x0a56, upmu_get_reg_value(0x0a56));
    print("[0x%x]=0x%x\n", 0x0a58, upmu_get_reg_value(0x0a58));	

    print("[0x%x]=0x%x\n", 0x0a7c, upmu_get_reg_value(0x0a7c));
    print("[0x%x]=0x%x\n", 0x0a7e, upmu_get_reg_value(0x0a7e));


	
    print("[0x%x]=0x%x\n", 0x0a60, upmu_get_reg_value(0x0a60));	
    print("[0x%x]=0x%x\n", 0x0a62, upmu_get_reg_value(0x0a62));	
    print("[0x%x]=0x%x\n", 0x0a66, upmu_get_reg_value(0x0a66));	
    print("[0x%x]=0x%x\n", 0x0a64, upmu_get_reg_value(0x0a64));		
	
    print("[0x%x]=0x%x\n", 0x0a72, upmu_get_reg_value(0x0a72));	
    print("[0x%x]=0x%x\n", 0x0a84, upmu_get_reg_value(0x0a84));	
    print("[0x%x]=0x%x\n", 0x0a7a, upmu_get_reg_value(0x0a7a));	
    print("[0x%x]=0x%x\n", 0x0a5c, upmu_get_reg_value(0x0a5c));		
    print("[0x%x]=0x%x\n", 0x0a6a, upmu_get_reg_value(0x0a6a));	
    print("[0x%x]=0x%x\n", 0x0a6c, upmu_get_reg_value(0x0a6c));	

    print("[0x%x]=0x%x\n", 0x043e, upmu_get_reg_value(0x043e));	
	
    print("[0x%x]=0x%x\n", 0x0470, upmu_get_reg_value(0x0470));	
    print("[0x%x]=0x%x\n", 0x046c, upmu_get_reg_value(0x046c));	
    print("[0x%x]=0x%x\n", 0x0466, upmu_get_reg_value(0x0466));	
    print("[0x%x]=0x%x\n", 0x0442, upmu_get_reg_value(0x0442));		
	
    print("[0x%x]=0x%x\n", 0x045a, upmu_get_reg_value(0x045a));	
    print("[0x%x]=0x%x\n", 0x0456, upmu_get_reg_value(0x0456));	

    print("[0x%x]=0x%x\n", 0x0450, upmu_get_reg_value(0x0450));	
    print("[0x%x]=0x%x\n", 0x044c, upmu_get_reg_value(0x044c));		

}



U32 efuse_data[0x20]={0};

void pmic_6325_efuse_check_bit(void)
{

	U32 val_reg=0;
	print("pmic_6328_efuse_check_bit\n");


pmic_read_interface(0x0434,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x0434,4,val_reg,(efuse_data[0]>>0)&1);
pmic_read_interface(0x0434,&val_reg,0x1,5); print("[0x%x:%d]:%d =%d  \n", 0x0434,5,val_reg,(efuse_data[0]>>1)&1);
pmic_read_interface(0x0434,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x0434,6,val_reg,(efuse_data[0]>>2)&1);
pmic_read_interface(0x0434,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x0434,7,val_reg,(efuse_data[0]>>3)&1);
pmic_read_interface(0x0434,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0434,8,val_reg,(efuse_data[0]>>4)&1);
pmic_read_interface(0x0434,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0434,9,val_reg,(efuse_data[0]>>5)&1);
pmic_read_interface(0x0434,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0434,10,val_reg,(efuse_data[0]>>6)&1);
pmic_read_interface(0x0434,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0434,11,val_reg,(efuse_data[0]>>7)&1);
pmic_read_interface(0x0434,&val_reg,0x1,12); print("[0x%x:%d]:%d =%d  \n", 0x0434,12,val_reg,(efuse_data[0]>>8)&1);
pmic_read_interface(0x0434,&val_reg,0x1,13); print("[0x%x:%d]:%d =%d  \n", 0x0434,13,val_reg,(efuse_data[0]>>9)&1);
pmic_read_interface(0x0438,&val_reg,0x1,0); print("[0x%x:%d]:%d =%d  \n", 0x0438,0,val_reg,(efuse_data[0]>>10)&1);
pmic_read_interface(0x0438,&val_reg,0x1,1); print("[0x%x:%d]:%d =%d  \n", 0x0438,1,val_reg,(efuse_data[0]>>11)&1);
pmic_read_interface(0x0438,&val_reg,0x1,2); print("[0x%x:%d]:%d =%d  \n", 0x0438,2,val_reg,(efuse_data[0]>>12)&1);
pmic_read_interface(0x0438,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x0438,3,val_reg,(efuse_data[0]>>13)&1);
pmic_read_interface(0x0438,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x0438,4,val_reg,(efuse_data[0]>>14)&1);
pmic_read_interface(0x0464,&val_reg,0x1,0); print("[0x%x:%d]:%d =%d  \n", 0x0464,0,val_reg,(efuse_data[0]>>15)&1);
pmic_read_interface(0x0464,&val_reg,0x1,1); print("[0x%x:%d]:%d =%d  \n", 0x0464,1,val_reg,(efuse_data[1]>>0)&1);
pmic_read_interface(0x0464,&val_reg,0x1,2); print("[0x%x:%d]:%d =%d  \n", 0x0464,2,val_reg,(efuse_data[1]>>1)&1);
pmic_read_interface(0x0464,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x0464,3,val_reg,(efuse_data[1]>>2)&1);
pmic_read_interface(0x0464,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x0464,4,val_reg,(efuse_data[1]>>3)&1);
pmic_read_interface(0x0464,&val_reg,0x1,5); print("[0x%x:%d]:%d =%d  \n", 0x0464,5,val_reg,(efuse_data[1]>>4)&1);
pmic_read_interface(0x0438,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0438,11,val_reg,(efuse_data[1]>>5)&1);
pmic_read_interface(0x0438,&val_reg,0x1,12); print("[0x%x:%d]:%d =%d  \n", 0x0438,12,val_reg,(efuse_data[1]>>6)&1);
pmic_read_interface(0x0438,&val_reg,0x1,13); print("[0x%x:%d]:%d =%d  \n", 0x0438,13,val_reg,(efuse_data[1]>>7)&1);
pmic_read_interface(0x0438,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0438,8,val_reg,(efuse_data[1]>>8)&1);
pmic_read_interface(0x0438,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0438,9,val_reg,(efuse_data[1]>>9)&1);
pmic_read_interface(0x0438,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0438,10,val_reg,(efuse_data[1]>>10)&1);
pmic_read_interface(0x0438,&val_reg,0x1,5); print("[0x%x:%d]:%d =%d  \n", 0x0438,5,val_reg,(efuse_data[1]>>11)&1);
pmic_read_interface(0x0438,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x0438,6,val_reg,(efuse_data[1]>>12)&1);
pmic_read_interface(0x0438,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x0438,7,val_reg,(efuse_data[1]>>13)&1);
pmic_read_interface(0x046E,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x046E,6,val_reg,(efuse_data[1]>>14)&1);
pmic_read_interface(0x046E,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x046E,7,val_reg,(efuse_data[1]>>15)&1);
pmic_read_interface(0x046E,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x046E,8,val_reg,(efuse_data[2]>>0)&1);
pmic_read_interface(0x046E,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x046E,9,val_reg,(efuse_data[2]>>1)&1);
pmic_read_interface(0x046E,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x046E,10,val_reg,(efuse_data[2]>>2)&1);
pmic_read_interface(0x046E,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x046E,11,val_reg,(efuse_data[2]>>3)&1);
pmic_read_interface(0x046E,&val_reg,0x1,0); print("[0x%x:%d]:%d =%d  \n", 0x046E,0,val_reg,(efuse_data[2]>>4)&1);
pmic_read_interface(0x046E,&val_reg,0x1,1); print("[0x%x:%d]:%d =%d  \n", 0x046E,1,val_reg,(efuse_data[2]>>5)&1);
pmic_read_interface(0x046E,&val_reg,0x1,2); print("[0x%x:%d]:%d =%d  \n", 0x046E,2,val_reg,(efuse_data[2]>>6)&1);
pmic_read_interface(0x046E,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x046E,3,val_reg,(efuse_data[2]>>7)&1);
pmic_read_interface(0x046E,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x046E,4,val_reg,(efuse_data[2]>>8)&1);
pmic_read_interface(0x046E,&val_reg,0x1,5); print("[0x%x:%d]:%d =%d  \n", 0x046E,5,val_reg,(efuse_data[2]>>9)&1);
pmic_read_interface(0x0444,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x0444,6,val_reg,(efuse_data[2]>>10)&1);
pmic_read_interface(0x0444,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x0444,7,val_reg,(efuse_data[2]>>11)&1);
pmic_read_interface(0x0444,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0444,8,val_reg,(efuse_data[2]>>12)&1);
pmic_read_interface(0x0444,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0444,9,val_reg,(efuse_data[2]>>13)&1);
pmic_read_interface(0x0444,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0444,10,val_reg,(efuse_data[2]>>14)&1);
pmic_read_interface(0x0444,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0444,11,val_reg,(efuse_data[2]>>15)&1);
pmic_read_interface(0x0444,&val_reg,0x1,0); print("[0x%x:%d]:%d =%d  \n", 0x0444,0,val_reg,(efuse_data[3]>>0)&1);
pmic_read_interface(0x0444,&val_reg,0x1,1); print("[0x%x:%d]:%d =%d  \n", 0x0444,1,val_reg,(efuse_data[3]>>1)&1);
pmic_read_interface(0x0444,&val_reg,0x1,2); print("[0x%x:%d]:%d =%d  \n", 0x0444,2,val_reg,(efuse_data[3]>>2)&1);
pmic_read_interface(0x0444,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x0444,3,val_reg,(efuse_data[3]>>3)&1);
pmic_read_interface(0x0444,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x0444,4,val_reg,(efuse_data[3]>>4)&1);
pmic_read_interface(0x0444,&val_reg,0x1,5); print("[0x%x:%d]:%d =%d  \n", 0x0444,5,val_reg,(efuse_data[3]>>5)&1);
pmic_read_interface(0x0458,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x0458,7,val_reg,(efuse_data[3]>>6)&1);
pmic_read_interface(0x0458,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0458,8,val_reg,(efuse_data[3]>>7)&1);
pmic_read_interface(0x0458,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0458,9,val_reg,(efuse_data[3]>>8)&1);
pmic_read_interface(0x0458,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0458,10,val_reg,(efuse_data[3]>>9)&1);
pmic_read_interface(0x0458,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0458,11,val_reg,(efuse_data[3]>>10)&1);
pmic_read_interface(0x0458,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x0458,6,val_reg,(efuse_data[3]>>11)&1);
pmic_read_interface(0x0458,&val_reg,0x1,1); print("[0x%x:%d]:%d =%d  \n", 0x0458,1,val_reg,(efuse_data[3]>>12)&1);
pmic_read_interface(0x0458,&val_reg,0x1,2); print("[0x%x:%d]:%d =%d  \n", 0x0458,2,val_reg,(efuse_data[3]>>13)&1);
pmic_read_interface(0x0458,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x0458,3,val_reg,(efuse_data[3]>>14)&1);
pmic_read_interface(0x0458,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x0458,4,val_reg,(efuse_data[3]>>15)&1);
pmic_read_interface(0x0458,&val_reg,0x1,5); print("[0x%x:%d]:%d =%d  \n", 0x0458,5,val_reg,(efuse_data[4]>>0)&1);
pmic_read_interface(0x0458,&val_reg,0x1,0); print("[0x%x:%d]:%d =%d  \n", 0x0458,0,val_reg,(efuse_data[4]>>1)&1);
pmic_read_interface(0x044E,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x044E,6,val_reg,(efuse_data[4]>>2)&1);
pmic_read_interface(0x044E,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x044E,7,val_reg,(efuse_data[4]>>3)&1);
pmic_read_interface(0x044E,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x044E,8,val_reg,(efuse_data[4]>>4)&1);
pmic_read_interface(0x044E,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x044E,9,val_reg,(efuse_data[4]>>5)&1);
pmic_read_interface(0x044E,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x044E,10,val_reg,(efuse_data[4]>>6)&1);
pmic_read_interface(0x044E,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x044E,11,val_reg,(efuse_data[4]>>7)&1);
pmic_read_interface(0x044E,&val_reg,0x1,0); print("[0x%x:%d]:%d =%d  \n", 0x044E,0,val_reg,(efuse_data[4]>>8)&1);
pmic_read_interface(0x044E,&val_reg,0x1,1); print("[0x%x:%d]:%d =%d  \n", 0x044E,1,val_reg,(efuse_data[4]>>9)&1);
pmic_read_interface(0x044E,&val_reg,0x1,2); print("[0x%x:%d]:%d =%d  \n", 0x044E,2,val_reg,(efuse_data[4]>>10)&1);
pmic_read_interface(0x044E,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x044E,3,val_reg,(efuse_data[4]>>11)&1);
pmic_read_interface(0x044E,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x044E,4,val_reg,(efuse_data[4]>>12)&1);
pmic_read_interface(0x044E,&val_reg,0x1,5); print("[0x%x:%d]:%d =%d  \n", 0x044E,5,val_reg,(efuse_data[4]>>13)&1);
pmic_read_interface(0x0A52,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A52,8,val_reg,(efuse_data[4]>>14)&1);
pmic_read_interface(0x0A52,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A52,9,val_reg,(efuse_data[4]>>15)&1);
pmic_read_interface(0x0A52,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A52,10,val_reg,(efuse_data[5]>>0)&1);
pmic_read_interface(0x0A52,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A52,11,val_reg,(efuse_data[5]>>1)&1);
pmic_read_interface(0x0A56,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A56,8,val_reg,(efuse_data[5]>>2)&1);
pmic_read_interface(0x0A56,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A56,9,val_reg,(efuse_data[5]>>3)&1);
pmic_read_interface(0x0A56,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A56,10,val_reg,(efuse_data[5]>>4)&1);
pmic_read_interface(0x0A56,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A56,11,val_reg,(efuse_data[5]>>5)&1);
pmic_read_interface(0x0A58,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A58,8,val_reg,(efuse_data[5]>>6)&1);
pmic_read_interface(0x0A58,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A58,9,val_reg,(efuse_data[5]>>7)&1);
pmic_read_interface(0x0A58,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A58,10,val_reg,(efuse_data[5]>>8)&1);
pmic_read_interface(0x0A58,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A58,11,val_reg,(efuse_data[5]>>9)&1);
pmic_read_interface(0x0A7C,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A7C,8,val_reg,(efuse_data[5]>>10)&1);
pmic_read_interface(0x0A7C,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A7C,9,val_reg,(efuse_data[5]>>11)&1);
pmic_read_interface(0x0A7C,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A7C,10,val_reg,(efuse_data[5]>>12)&1);
pmic_read_interface(0x0A7C,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A7C,11,val_reg,(efuse_data[5]>>13)&1);
pmic_read_interface(0x0A7E,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A7E,8,val_reg,(efuse_data[5]>>14)&1);
pmic_read_interface(0x0A7E,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A7E,9,val_reg,(efuse_data[5]>>15)&1);
pmic_read_interface(0x0A7E,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A7E,10,val_reg,(efuse_data[6]>>0)&1);
pmic_read_interface(0x0A7E,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A7E,11,val_reg,(efuse_data[6]>>1)&1);
pmic_read_interface(0x0A60,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A60,8,val_reg,(efuse_data[6]>>2)&1);
pmic_read_interface(0x0A60,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A60,9,val_reg,(efuse_data[6]>>3)&1);
pmic_read_interface(0x0A60,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A60,10,val_reg,(efuse_data[6]>>4)&1);
pmic_read_interface(0x0A60,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A60,11,val_reg,(efuse_data[6]>>5)&1);
pmic_read_interface(0x0A62,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A62,8,val_reg,(efuse_data[6]>>6)&1);
pmic_read_interface(0x0A62,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A62,9,val_reg,(efuse_data[6]>>7)&1);
pmic_read_interface(0x0A62,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A62,10,val_reg,(efuse_data[6]>>8)&1);
pmic_read_interface(0x0A62,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A62,11,val_reg,(efuse_data[6]>>9)&1);
pmic_read_interface(0x0A66,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A66,8,val_reg,(efuse_data[6]>>10)&1);
pmic_read_interface(0x0A66,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A66,9,val_reg,(efuse_data[6]>>11)&1);
pmic_read_interface(0x0A66,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A66,10,val_reg,(efuse_data[6]>>12)&1);
pmic_read_interface(0x0A66,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A66,11,val_reg,(efuse_data[6]>>13)&1);
pmic_read_interface(0x0A64,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A64,8,val_reg,(efuse_data[6]>>14)&1);
pmic_read_interface(0x0A64,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A64,9,val_reg,(efuse_data[6]>>15)&1);
pmic_read_interface(0x0A64,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A64,10,val_reg,(efuse_data[7]>>0)&1);
pmic_read_interface(0x0A64,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A64,11,val_reg,(efuse_data[7]>>1)&1);
pmic_read_interface(0x0A72,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A72,8,val_reg,(efuse_data[7]>>2)&1);
pmic_read_interface(0x0A72,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A72,9,val_reg,(efuse_data[7]>>3)&1);
pmic_read_interface(0x0A72,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A72,10,val_reg,(efuse_data[7]>>4)&1);
pmic_read_interface(0x0A72,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A72,11,val_reg,(efuse_data[7]>>5)&1);
pmic_read_interface(0x0A84,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A84,8,val_reg,(efuse_data[7]>>6)&1);
pmic_read_interface(0x0A84,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A84,9,val_reg,(efuse_data[7]>>7)&1);
pmic_read_interface(0x0A84,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A84,10,val_reg,(efuse_data[7]>>8)&1);
pmic_read_interface(0x0A84,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A84,11,val_reg,(efuse_data[7]>>9)&1);
pmic_read_interface(0x0A7A,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A7A,8,val_reg,(efuse_data[7]>>10)&1);
pmic_read_interface(0x0A7A,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A7A,9,val_reg,(efuse_data[7]>>11)&1);
pmic_read_interface(0x0A7A,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A7A,10,val_reg,(efuse_data[7]>>12)&1);
pmic_read_interface(0x0A7A,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A7A,11,val_reg,(efuse_data[7]>>13)&1);
pmic_read_interface(0x0A5C,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A5C,9,val_reg,(efuse_data[7]>>14)&1);
pmic_read_interface(0x0A5C,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A5C,10,val_reg,(efuse_data[7]>>15)&1);
pmic_read_interface(0x0A5C,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A5C,11,val_reg,(efuse_data[8]>>0)&1);
pmic_read_interface(0x0A5C,&val_reg,0x1,12); print("[0x%x:%d]:%d =%d  \n", 0x0A5C,12,val_reg,(efuse_data[8]>>1)&1);
pmic_read_interface(0x0A6A,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A6A,8,val_reg,(efuse_data[8]>>2)&1);
pmic_read_interface(0x0A6A,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A6A,9,val_reg,(efuse_data[8]>>3)&1);
pmic_read_interface(0x0A6A,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A6A,10,val_reg,(efuse_data[8]>>4)&1);
pmic_read_interface(0x0A6A,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A6A,11,val_reg,(efuse_data[8]>>5)&1);
pmic_read_interface(0x0A6C,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x0A6C,8,val_reg,(efuse_data[8]>>6)&1);
pmic_read_interface(0x0A6C,&val_reg,0x1,9); print("[0x%x:%d]:%d =%d  \n", 0x0A6C,9,val_reg,(efuse_data[8]>>7)&1);
pmic_read_interface(0x0A6C,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x0A6C,10,val_reg,(efuse_data[8]>>8)&1);
pmic_read_interface(0x0A6C,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x0A6C,11,val_reg,(efuse_data[8]>>9)&1);
pmic_read_interface(0x043E,&val_reg,0x1,10); print("[0x%x:%d]:%d =%d  \n", 0x043E,10,val_reg,(efuse_data[8]>>10)&1);
pmic_read_interface(0x043E,&val_reg,0x1,11); print("[0x%x:%d]:%d =%d  \n", 0x043E,11,val_reg,(efuse_data[8]>>11)&1);
pmic_read_interface(0x043E,&val_reg,0x1,12); print("[0x%x:%d]:%d =%d  \n", 0x043E,12,val_reg,(efuse_data[8]>>12)&1);

pmic_read_interface(0x0470,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x0470,6,val_reg,(efuse_data[12]>>7)&1);
pmic_read_interface(0x0470,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x0470,7,val_reg,(efuse_data[12]>>8)&1);
pmic_read_interface(0x046C,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x046C,3,val_reg,(efuse_data[12]>>9)&1);
pmic_read_interface(0x046C,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x046C,4,val_reg,(efuse_data[12]>>10)&1);
pmic_read_interface(0x0466,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x0466,6,val_reg,(efuse_data[12]>>11)&1);
pmic_read_interface(0x0466,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x0466,7,val_reg,(efuse_data[12]>>12)&1);
pmic_read_interface(0x0442,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x0442,3,val_reg,(efuse_data[12]>>13)&1);
pmic_read_interface(0x0442,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x0442,4,val_reg,(efuse_data[12]>>14)&1);
pmic_read_interface(0x045A,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x045A,6,val_reg,(efuse_data[12]>>15)&1);
pmic_read_interface(0x045A,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x045A,7,val_reg,(efuse_data[13]>>0)&1);
pmic_read_interface(0x0456,&val_reg,0x1,3); print("[0x%x:%d]:%d =%d  \n", 0x0456,3,val_reg,(efuse_data[13]>>1)&1);
pmic_read_interface(0x0456,&val_reg,0x1,4); print("[0x%x:%d]:%d =%d  \n", 0x0456,4,val_reg,(efuse_data[13]>>2)&1);
pmic_read_interface(0x0450,&val_reg,0x1,6); print("[0x%x:%d]:%d =%d  \n", 0x0450,6,val_reg,(efuse_data[13]>>3)&1);
pmic_read_interface(0x0450,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x0450,7,val_reg,(efuse_data[13]>>4)&1);
pmic_read_interface(0x044C,&val_reg,0x1,7); print("[0x%x:%d]:%d =%d  \n", 0x044C,7,val_reg,(efuse_data[13]>>5)&1);
pmic_read_interface(0x044C,&val_reg,0x1,8); print("[0x%x:%d]:%d =%d  \n", 0x044C,8,val_reg,(efuse_data[13]>>6)&1);

}

void pmic_6328_efuse_management(void)
{
    
    int i=0;
    int is_efuse_trimed=0;

    is_efuse_trimed = ((upmu_get_reg_value(0xC5C))>>15)&0x0001;

    print("[6328] is_efuse_trimed=0x%x,[0x%x]=0x%x\n", is_efuse_trimed, 0xC5C, upmu_get_reg_value(0xC5C));

    if(is_efuse_trimed == 1)
    {
        //get efuse data
	//turn on efuse clock
	pmic_config_interface(MT6328_PMIC_RG_EFUSE_CK_PDN_HWEN_ADDR, 0x00, MT6328_PMIC_RG_EFUSE_CK_PDN_HWEN_MASK, MT6328_PMIC_RG_EFUSE_CK_PDN_HWEN_SHIFT);
	pmic_config_interface(MT6328_PMIC_RG_EFUSE_CK_PDN_ADDR, 0x00, MT6328_PMIC_RG_EFUSE_CK_PDN_MASK, MT6328_PMIC_RG_EFUSE_CK_PDN_SHIFT);
	pmic_config_interface(MT6328_PMIC_RG_OTP_RD_SW_ADDR, 0x01, MT6328_PMIC_RG_OTP_RD_SW_MASK, MT6328_PMIC_RG_OTP_RD_SW_SHIFT);
	
	for(i=0;i<14;i++)
	{
		efuse_data[i]=pmic_read_efuse(i);
	}

	//turn off efuse clock
	pmic_config_interface(MT6328_PMIC_RG_EFUSE_CK_PDN_HWEN_ADDR, 0x01, MT6328_PMIC_RG_EFUSE_CK_PDN_HWEN_MASK, MT6328_PMIC_RG_EFUSE_CK_PDN_HWEN_SHIFT);
	pmic_config_interface(MT6328_PMIC_RG_EFUSE_CK_PDN_ADDR, 0x01, MT6328_PMIC_RG_EFUSE_CK_PDN_MASK, MT6328_PMIC_RG_EFUSE_CK_PDN_SHIFT);

        
        //dump efuse data for check
        for(i=0x0;i<=0x1F;i++)
            print("[6328] efuse_data[0x%x]=0x%x\n", i, efuse_data[i]);

    
        //print("Before apply pmic efuse\n");
        //pmic_6328_efuse_check();

        //------------------------------------------
	pmic_config_interface(0x0434,((efuse_data[0] >>0 )&0x0001),0x1,4);
	pmic_config_interface(0x0434,((efuse_data[0] >>1 )&0x0001),0x1,5);
	pmic_config_interface(0x0434,((efuse_data[0] >>2 )&0x0001),0x1,6);
	pmic_config_interface(0x0434,((efuse_data[0] >>3 )&0x0001),0x1,7);
	pmic_config_interface(0x0434,((efuse_data[0] >>4 )&0x0001),0x1,8);
	pmic_config_interface(0x0434,((efuse_data[0] >>5 )&0x0001),0x1,9);
	pmic_config_interface(0x0434,((efuse_data[0] >>6 )&0x0001),0x1,10);
	pmic_config_interface(0x0434,((efuse_data[0] >>7 )&0x0001),0x1,11);
	pmic_config_interface(0x0434,((efuse_data[0] >>8 )&0x0001),0x1,12);
	pmic_config_interface(0x0434,((efuse_data[0] >>9 )&0x0001),0x1,13);
	pmic_config_interface(0x0438,((efuse_data[0] >>10 )&0x0001),0x1,0);
	pmic_config_interface(0x0438,((efuse_data[0] >>11 )&0x0001),0x1,1);
	pmic_config_interface(0x0438,((efuse_data[0] >>12 )&0x0001),0x1,2);
	pmic_config_interface(0x0438,((efuse_data[0] >>13 )&0x0001),0x1,3);
	pmic_config_interface(0x0438,((efuse_data[0] >>14 )&0x0001),0x1,4);
	pmic_config_interface(0x0464,((efuse_data[0] >>15 )&0x0001),0x1,0);
	pmic_config_interface(0x0464,((efuse_data[1] >>0 )&0x0001),0x1,1);
	pmic_config_interface(0x0464,((efuse_data[1] >>1 )&0x0001),0x1,2);
	pmic_config_interface(0x0464,((efuse_data[1] >>2 )&0x0001),0x1,3);
	pmic_config_interface(0x0464,((efuse_data[1] >>3 )&0x0001),0x1,4);
	pmic_config_interface(0x0464,((efuse_data[1] >>4 )&0x0001),0x1,5);
	pmic_config_interface(0x0438,((efuse_data[1] >>5 )&0x0001),0x1,11);
	pmic_config_interface(0x0438,((efuse_data[1] >>6 )&0x0001),0x1,12);
	pmic_config_interface(0x0438,((efuse_data[1] >>7 )&0x0001),0x1,13);
	pmic_config_interface(0x0438,((efuse_data[1] >>8 )&0x0001),0x1,8);
	pmic_config_interface(0x0438,((efuse_data[1] >>9 )&0x0001),0x1,9);
	pmic_config_interface(0x0438,((efuse_data[1] >>10 )&0x0001),0x1,10);
	pmic_config_interface(0x0438,((efuse_data[1] >>11 )&0x0001),0x1,5);
	pmic_config_interface(0x0438,((efuse_data[1] >>12 )&0x0001),0x1,6);
	pmic_config_interface(0x0438,((efuse_data[1] >>13 )&0x0001),0x1,7);
	pmic_config_interface(0x046E,((efuse_data[1] >>14 )&0x0001),0x1,6);
	pmic_config_interface(0x046E,((efuse_data[1] >>15 )&0x0001),0x1,7);
	pmic_config_interface(0x046E,((efuse_data[2] >>0 )&0x0001),0x1,8);
	pmic_config_interface(0x046E,((efuse_data[2] >>1 )&0x0001),0x1,9);
	pmic_config_interface(0x046E,((efuse_data[2] >>2 )&0x0001),0x1,10);
	pmic_config_interface(0x046E,((efuse_data[2] >>3 )&0x0001),0x1,11);
	pmic_config_interface(0x046E,((efuse_data[2] >>4 )&0x0001),0x1,0);
	pmic_config_interface(0x046E,((efuse_data[2] >>5 )&0x0001),0x1,1);
	pmic_config_interface(0x046E,((efuse_data[2] >>6 )&0x0001),0x1,2);
	pmic_config_interface(0x046E,((efuse_data[2] >>7 )&0x0001),0x1,3);
	pmic_config_interface(0x046E,((efuse_data[2] >>8 )&0x0001),0x1,4);
	pmic_config_interface(0x046E,((efuse_data[2] >>9 )&0x0001),0x1,5);
	pmic_config_interface(0x0444,((efuse_data[2] >>10 )&0x0001),0x1,6);
	pmic_config_interface(0x0444,((efuse_data[2] >>11 )&0x0001),0x1,7);
	pmic_config_interface(0x0444,((efuse_data[2] >>12 )&0x0001),0x1,8);
	pmic_config_interface(0x0444,((efuse_data[2] >>13 )&0x0001),0x1,9);
	pmic_config_interface(0x0444,((efuse_data[2] >>14 )&0x0001),0x1,10);
	pmic_config_interface(0x0444,((efuse_data[2] >>15 )&0x0001),0x1,11);
	pmic_config_interface(0x0444,((efuse_data[3] >>0 )&0x0001),0x1,0);
	pmic_config_interface(0x0444,((efuse_data[3] >>1 )&0x0001),0x1,1);
	pmic_config_interface(0x0444,((efuse_data[3] >>2 )&0x0001),0x1,2);
	pmic_config_interface(0x0444,((efuse_data[3] >>3 )&0x0001),0x1,3);
	pmic_config_interface(0x0444,((efuse_data[3] >>4 )&0x0001),0x1,4);
	pmic_config_interface(0x0444,((efuse_data[3] >>5 )&0x0001),0x1,5);
	pmic_config_interface(0x0458,((efuse_data[3] >>6 )&0x0001),0x1,7);
	pmic_config_interface(0x0458,((efuse_data[3] >>7 )&0x0001),0x1,8);
	pmic_config_interface(0x0458,((efuse_data[3] >>8 )&0x0001),0x1,9);
	pmic_config_interface(0x0458,((efuse_data[3] >>9 )&0x0001),0x1,10);
	pmic_config_interface(0x0458,((efuse_data[3] >>10 )&0x0001),0x1,11);
	pmic_config_interface(0x0458,((efuse_data[3] >>11 )&0x0001),0x1,6);
	pmic_config_interface(0x0458,((efuse_data[3] >>12 )&0x0001),0x1,1);
	pmic_config_interface(0x0458,((efuse_data[3] >>13 )&0x0001),0x1,2);
	pmic_config_interface(0x0458,((efuse_data[3] >>14 )&0x0001),0x1,3);
	pmic_config_interface(0x0458,((efuse_data[3] >>15 )&0x0001),0x1,4);
	pmic_config_interface(0x0458,((efuse_data[4] >>0 )&0x0001),0x1,5);
	pmic_config_interface(0x0458,((efuse_data[4] >>1 )&0x0001),0x1,0);
	pmic_config_interface(0x044E,((efuse_data[4] >>2 )&0x0001),0x1,6);
	pmic_config_interface(0x044E,((efuse_data[4] >>3 )&0x0001),0x1,7);
	pmic_config_interface(0x044E,((efuse_data[4] >>4 )&0x0001),0x1,8);
	pmic_config_interface(0x044E,((efuse_data[4] >>5 )&0x0001),0x1,9);
	pmic_config_interface(0x044E,((efuse_data[4] >>6 )&0x0001),0x1,10);
	pmic_config_interface(0x044E,((efuse_data[4] >>7 )&0x0001),0x1,11);
	pmic_config_interface(0x044E,((efuse_data[4] >>8 )&0x0001),0x1,0);
	pmic_config_interface(0x044E,((efuse_data[4] >>9 )&0x0001),0x1,1);
	pmic_config_interface(0x044E,((efuse_data[4] >>10 )&0x0001),0x1,2);
	pmic_config_interface(0x044E,((efuse_data[4] >>11 )&0x0001),0x1,3);
	pmic_config_interface(0x044E,((efuse_data[4] >>12 )&0x0001),0x1,4);
	pmic_config_interface(0x044E,((efuse_data[4] >>13 )&0x0001),0x1,5);
	pmic_config_interface(0x0A52,((efuse_data[4] >>14 )&0x0001),0x1,8);
	pmic_config_interface(0x0A52,((efuse_data[4] >>15 )&0x0001),0x1,9);
	pmic_config_interface(0x0A52,((efuse_data[5] >>0 )&0x0001),0x1,10);
	pmic_config_interface(0x0A52,((efuse_data[5] >>1 )&0x0001),0x1,11);
	pmic_config_interface(0x0A56,((efuse_data[5] >>2 )&0x0001),0x1,8);
	pmic_config_interface(0x0A56,((efuse_data[5] >>3 )&0x0001),0x1,9);
	pmic_config_interface(0x0A56,((efuse_data[5] >>4 )&0x0001),0x1,10);
	pmic_config_interface(0x0A56,((efuse_data[5] >>5 )&0x0001),0x1,11);
	pmic_config_interface(0x0A58,((efuse_data[5] >>6 )&0x0001),0x1,8);
	pmic_config_interface(0x0A58,((efuse_data[5] >>7 )&0x0001),0x1,9);
	pmic_config_interface(0x0A58,((efuse_data[5] >>8 )&0x0001),0x1,10);
	pmic_config_interface(0x0A58,((efuse_data[5] >>9 )&0x0001),0x1,11);
	pmic_config_interface(0x0A7C,((efuse_data[5] >>10 )&0x0001),0x1,8);
	pmic_config_interface(0x0A7C,((efuse_data[5] >>11 )&0x0001),0x1,9);
	pmic_config_interface(0x0A7C,((efuse_data[5] >>12 )&0x0001),0x1,10);
	pmic_config_interface(0x0A7C,((efuse_data[5] >>13 )&0x0001),0x1,11);
	pmic_config_interface(0x0A7E,((efuse_data[5] >>14 )&0x0001),0x1,8);
	pmic_config_interface(0x0A7E,((efuse_data[5] >>15 )&0x0001),0x1,9);
	pmic_config_interface(0x0A7E,((efuse_data[6] >>0 )&0x0001),0x1,10);
	pmic_config_interface(0x0A7E,((efuse_data[6] >>1 )&0x0001),0x1,11);
	pmic_config_interface(0x0A60,((efuse_data[6] >>2 )&0x0001),0x1,8);
	pmic_config_interface(0x0A60,((efuse_data[6] >>3 )&0x0001),0x1,9);
	pmic_config_interface(0x0A60,((efuse_data[6] >>4 )&0x0001),0x1,10);
	pmic_config_interface(0x0A60,((efuse_data[6] >>5 )&0x0001),0x1,11);
	pmic_config_interface(0x0A62,((efuse_data[6] >>6 )&0x0001),0x1,8);
	pmic_config_interface(0x0A62,((efuse_data[6] >>7 )&0x0001),0x1,9);
	pmic_config_interface(0x0A62,((efuse_data[6] >>8 )&0x0001),0x1,10);
	pmic_config_interface(0x0A62,((efuse_data[6] >>9 )&0x0001),0x1,11);
	pmic_config_interface(0x0A66,((efuse_data[6] >>10 )&0x0001),0x1,8);
	pmic_config_interface(0x0A66,((efuse_data[6] >>11 )&0x0001),0x1,9);
	pmic_config_interface(0x0A66,((efuse_data[6] >>12 )&0x0001),0x1,10);
	pmic_config_interface(0x0A66,((efuse_data[6] >>13 )&0x0001),0x1,11);
	pmic_config_interface(0x0A64,((efuse_data[6] >>14 )&0x0001),0x1,8);
	pmic_config_interface(0x0A64,((efuse_data[6] >>15 )&0x0001),0x1,9);
	pmic_config_interface(0x0A64,((efuse_data[7] >>0 )&0x0001),0x1,10);
	pmic_config_interface(0x0A64,((efuse_data[7] >>1 )&0x0001),0x1,11);
	pmic_config_interface(0x0A72,((efuse_data[7] >>2 )&0x0001),0x1,8);
	pmic_config_interface(0x0A72,((efuse_data[7] >>3 )&0x0001),0x1,9);
	pmic_config_interface(0x0A72,((efuse_data[7] >>4 )&0x0001),0x1,10);
	pmic_config_interface(0x0A72,((efuse_data[7] >>5 )&0x0001),0x1,11);
	pmic_config_interface(0x0A84,((efuse_data[7] >>6 )&0x0001),0x1,8);
	pmic_config_interface(0x0A84,((efuse_data[7] >>7 )&0x0001),0x1,9);
	pmic_config_interface(0x0A84,((efuse_data[7] >>8 )&0x0001),0x1,10);
	pmic_config_interface(0x0A84,((efuse_data[7] >>9 )&0x0001),0x1,11);
	pmic_config_interface(0x0A7A,((efuse_data[7] >>10 )&0x0001),0x1,8);
	pmic_config_interface(0x0A7A,((efuse_data[7] >>11 )&0x0001),0x1,9);
	pmic_config_interface(0x0A7A,((efuse_data[7] >>12 )&0x0001),0x1,10);
	pmic_config_interface(0x0A7A,((efuse_data[7] >>13 )&0x0001),0x1,11);
	pmic_config_interface(0x0A5C,((efuse_data[7] >>14 )&0x0001),0x1,9);
	pmic_config_interface(0x0A5C,((efuse_data[7] >>15 )&0x0001),0x1,10);
	pmic_config_interface(0x0A5C,((efuse_data[8] >>0 )&0x0001),0x1,11);
	pmic_config_interface(0x0A5C,((efuse_data[8] >>1 )&0x0001),0x1,12);
	pmic_config_interface(0x0A6A,((efuse_data[8] >>2 )&0x0001),0x1,8);
	pmic_config_interface(0x0A6A,((efuse_data[8] >>3 )&0x0001),0x1,9);
	pmic_config_interface(0x0A6A,((efuse_data[8] >>4 )&0x0001),0x1,10);
	pmic_config_interface(0x0A6A,((efuse_data[8] >>5 )&0x0001),0x1,11);
	pmic_config_interface(0x0A6C,((efuse_data[8] >>6 )&0x0001),0x1,8);
	pmic_config_interface(0x0A6C,((efuse_data[8] >>7 )&0x0001),0x1,9);
	pmic_config_interface(0x0A6C,((efuse_data[8] >>8 )&0x0001),0x1,10);
	pmic_config_interface(0x0A6C,((efuse_data[8] >>9 )&0x0001),0x1,11);
	pmic_config_interface(0x043E,((efuse_data[8] >>10 )&0x0001),0x1,10);
	pmic_config_interface(0x043E,((efuse_data[8] >>11 )&0x0001),0x1,11);
	pmic_config_interface(0x043E,((efuse_data[8] >>12 )&0x0001),0x1,12);


	pmic_config_interface(0x0470,((efuse_data[12] >>7 )&0x0001),0x1,6);
	pmic_config_interface(0x0470,((efuse_data[12] >>8 )&0x0001),0x1,7);
	pmic_config_interface(0x046C,((efuse_data[12] >>9 )&0x0001),0x1,3);
	pmic_config_interface(0x046C,((efuse_data[12] >>10 )&0x0001),0x1,4);
	pmic_config_interface(0x0466,((efuse_data[12] >>11 )&0x0001),0x1,6);
	pmic_config_interface(0x0466,((efuse_data[12] >>12 )&0x0001),0x1,7);
	pmic_config_interface(0x0442,((efuse_data[12] >>13 )&0x0001),0x1,3);
	pmic_config_interface(0x0442,((efuse_data[12] >>14 )&0x0001),0x1,4);
	pmic_config_interface(0x045A,((efuse_data[12] >>15 )&0x0001),0x1,6);
	pmic_config_interface(0x045A,((efuse_data[13] >>0 )&0x0001),0x1,7);
	pmic_config_interface(0x0456,((efuse_data[13] >>1 )&0x0001),0x1,3);
	pmic_config_interface(0x0456,((efuse_data[13] >>2 )&0x0001),0x1,4);
	pmic_config_interface(0x0450,((efuse_data[13] >>3 )&0x0001),0x1,6);
	pmic_config_interface(0x0450,((efuse_data[13] >>4 )&0x0001),0x1,7);
	pmic_config_interface(0x044C,((efuse_data[13] >>5 )&0x0001),0x1,7);
	pmic_config_interface(0x044C,((efuse_data[13] >>6 )&0x0001),0x1,8);


        //------------------------------------------

        //print("After apply pmic efuse\n");
        //pmic_6328_efuse_check();

        //pmic_6325_efuse_check_bit();
    }
}

const static unsigned char mt6328_VIO_1_84[] = {
	14,	15,	0,	1,	2,	3,	4,	5,	8,	8,	8,	9,	10,	11,	12,	13
};

static unsigned char vio18_cal;

void upmu_set_rg_vio18_184(void)
{
	print("[upmu_set_rg_vio18_184] old cal=%d new cal=%d.\r\n", vio18_cal,mt6328_VIO_1_84[vio18_cal]);
	pmic_config_interface(MT6328_PMIC_RG_VIO18_CAL_ADDR, mt6328_VIO_1_84[vio18_cal], MT6328_PMIC_RG_VIO18_CAL_MASK, MT6328_PMIC_RG_VIO18_CAL_SHIFT);
    }

/*
const static unsigned char mt6328_VMC_1_86[] = {
	13,	14,	15,	0,	1,	2,	3,	4,	5,	8,	8,	9,	10,	11,	12,	13
};
	

static unsigned char vmc_cal;

void upmu_set_rg_vmc_186(void)
{
	print("[upmu_set_rg_vio18_184] old cal=%d new cal=%d.\r\n", vmc_cal,mt6328_VMC_1_86[vmc_cal]);
	pmic_config_interface(MT6328_PMIC_RG_VMC_CAL_ADDR, mt6328_VMC_1_86[vmc_cal], MT6328_PMIC_RG_VMC_CAL_MASK, MT6328_PMIC_RG_VMC_CAL_SHIFT);
}
*/


void pmic_disable_usbdl_wo_battery(void)
{
	print("[pmic_init] turn off usbdl wo battery..................\n");   
    pmic_config_interface(MT6328_PMIC_RG_USBDL_SET_ADDR, 0x0000, MT6328_PMIC_RG_USBDL_SET_MASK, MT6328_PMIC_RG_USBDL_SET_SHIFT); //[1]=0, RG_WDTRSTB_MODE
    pmic_config_interface(MT6328_PMIC_RG_USBDL_RST_ADDR, 0x0001, MT6328_PMIC_RG_USBDL_RST_MASK, MT6328_PMIC_RG_USBDL_RST_SHIFT); //[0]=1, RG_WDTRSTB_EN

}


//==============================================================================
// PMIC Init Code
//==============================================================================
U32 pmic_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;
    int ret_val=0;
    int reg_val=0;

    ret_val=pmic_config_interface(MT6328_PMIC_BIAS_GEN_EN_SEL_ADDR, 0x0001, MT6328_PMIC_BIAS_GEN_EN_SEL_MASK, MT6328_PMIC_BIAS_GEN_EN_SEL_SHIFT); 
    ret_val=pmic_config_interface(MT6328_PMIC_BIAS_GEN_EN_ADDR, 0x0000, MT6328_PMIC_BIAS_GEN_EN_MASK, MT6328_PMIC_BIAS_GEN_EN_SHIFT); 

    print("[pmic_init] Preloader Start..................\n");    
    print("[pmic_init] MT6328 CHIP Code = 0x%x\n", get_MT6328_PMIC_chip_version());

	if(hw_check_battery()==1)
	{
		pmic_disable_usbdl_wo_battery();
	}

	//detect V battery Drop 
	pmic_DetectVbatDrop();

    pmic_6328_efuse_management();
        
	pmic_read_interface(MT6328_PMIC_RG_VIO18_CAL_ADDR, &vio18_cal, MT6328_PMIC_RG_VIO18_CAL_MASK, MT6328_PMIC_RG_VIO18_CAL_SHIFT);
	upmu_set_rg_vio18_184();
	//pmic_read_interface(MT6328_PMIC_RG_VMC_CAL_ADDR, &vmc_cal, MT6328_PMIC_RG_VMC_CAL_MASK, MT6328_PMIC_RG_VMC_CAL_SHIFT);
	//upmu_set_rg_vmc_186();
	
    #if 1
    //Enable PMIC RST function (depends on main chip RST function)
    ret_val=pmic_config_interface(MT6328_TOP_RST_MISC_CLR, 0x0002, 0xFFFF, 0); //[1]=0, RG_WDTRSTB_MODE
    ret_val=pmic_config_interface(MT6328_TOP_RST_MISC_SET, 0x0001, 0xFFFF, 0); //[0]=1, RG_WDTRSTB_EN
    print("[pmic_init] Reg[0x%x]=0x%x\n", MT6328_TOP_RST_MISC, upmu_get_reg_value(MT6328_TOP_RST_MISC));        
    #endif


	 ret_val= pmic_config_interface(MT6328_PMIC_RG_SMPS_TESTMODE_B_ADDR, 0x0001, MT6328_PMIC_RG_SMPS_TESTMODE_B_MASK, MT6328_PMIC_RG_SMPS_TESTMODE_B_SHIFT); //RG_SMPS_TESTMODE_B by luke
	 
     ret_val = pmic_config_interface(0xA44,0x1,0x1,1); // [1:1]: RG_TREF_EN; Tim

	upmu_set_reg_value(0x4b4,0x68); //vsram 1.25v
	upmu_set_reg_value(0x4b8,0x68); //vsram 1.25v

    #ifdef DUMMY_AP
    //print("[pmic_init for DUMMY_AP]\n");
    #endif

    print("[pmic_init] Done...................\n");

    return ret_code;
}

