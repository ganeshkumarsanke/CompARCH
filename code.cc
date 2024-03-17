#include "predictor.h"

uint16_t local_history[1024]	={0};
uint16_t local_prediction[1024] ={0};
uint16_t global_prediction[4096]={0};
uint16_t choice_prediction[4096]={0};
uint16_t path_history;

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os) 
{
uint16_t mux_sel 	= choice_prediction[path_history % 4096];
uint16_t pc		= (br->instruction_addr >> 2) % 1024;
bool conditional_check	= br->is_conditional;
uint16_t local_pred_indx= local_history[pc] % 1024;
uint16_t path_hist_value= path_history % 4096;
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
			prediction = (global_prediction[path_hist_value] > 1);
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
uint16_t pc 		  = (br->instruction_addr >> 2) % 1024;
uint16_t local_pred_indx  = local_history[pc] % 1024;
uint16_t path_hist_value  = path_history % 4096;
bool conditional_check	  = br->is_conditional;
bool local_pred 	  = (local_prediction[local_pred_indx] > 3);
bool global_pred          = (global_prediction[path_hist_value] > 1);
    
if(conditional_check)
{
path_history = (path_history << 1) | taken;
if (global_pred == taken && local_pred != taken)
{
	choice_prediction[path_hist_value] += (choice_prediction[path_hist_value] < 3);
} 
else if (global_pred != taken && local_pred == taken)
{
        choice_prediction[path_hist_value] -= (choice_prediction[path_hist_value] > 0);
}
if (taken)
{
        global_prediction[path_hist_value]  +=(global_prediction[path_hist_value] < 3);
	local_prediction[local_pred_indx]   +=(local_prediction[local_pred_indx] < 7);
} 
else
{
	global_prediction[path_hist_value]  -=(global_prediction[path_hist_value] > 0);	
        local_prediction[local_pred_indx]   -=(local_prediction[local_pred_indx] > 0);
}
local_history[pc] = (local_history[pc] << 1) | taken;
}
else
{
	path_history = (path_history << 1) | taken;
}
}
