#include "predictor.h"

uint16_t local_history[1024]	={0};
uint16_t local_prediction[1024] ={0};
uint16_t global_prediction[4096]={0};
uint16_t choice_prediction[4096]={0};
uint16_t path_history;

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
uint16_t path_hist_value= path_history % 4096;

uint16_t pc_g		= (br->instruction_addr >> 2) % 4096;
uint16_t pc_l		= pc_g & 0x3FF;

uint16_t indx = path_hist_value ^ pc_g;
uint16_t mux_sel= choice_prediction[indx];

bool conditional_check	= br->is_conditional;
uint16_t local_pred_indx= local_history[pc_l] % 1024;

bool prediction;

if(conditional_check)
{
	switch(mux_sel)
	{
		case 0:
		case 1:
			prediction = (local_prediction[local_pred_indx] > 3);
			break;
		case 2:
		case 3:
			prediction = (global_prediction[indx] > 1);
			break;
		default:
			prediction = true;
			break;
	}
}
else
{
	prediction = true;
}
return prediction;
}


void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken) 
{
	uint16_t path_hist_value  = path_history % 4096;
uint16_t pc_g		= (br->instruction_addr >> 2) % 4096;
uint16_t pc_l		= pc_g & 0x3FF;
uint16_t local_pred_indx  = local_history[pc_l] % 1024;
bool conditional_check	  = br->is_conditional;
uint16_t indx = path_hist_value ^ pc_g;

bool local_pred 	  	  = (local_prediction[local_pred_indx] > 3);
bool global_pred          = (global_prediction[indx] > 1);
    
if(conditional_check)
{
	path_history = (path_history << 1) | taken;
	
	if (global_pred == taken && local_pred != taken)
	{
		choice_prediction[indx] += (choice_prediction[indx] < 3);
	} 
	else if (global_pred != taken && local_pred == taken)
	{
		choice_prediction[indx] -= (choice_prediction[indx] > 0);
	}
	
	if (taken)
	{
		global_prediction[indx]  +=(global_prediction[indx] < 3);
		local_prediction[local_pred_indx]   +=(local_prediction[local_pred_indx] < 7);
	} 
	else
	{
		global_prediction[indx]  -=(global_prediction[indx] > 0);	
		local_prediction[local_pred_indx]   -=(local_prediction[local_pred_indx] > 0);
	}
	local_history[pc_l] = (local_history[pc_l] << 1) | taken;
	}
else
{
	path_history = (path_history << 1) | taken;
}
}
