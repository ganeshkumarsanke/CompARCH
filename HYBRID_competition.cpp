#include "predictor.h"

uint16_t Bimodal[1024] ={0};
uint16_t Gshare[8192]={0};
uint16_t Select_prediction[1024]={0};
uint16_t GHR;

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
uint16_t path_hist_value= GHR % 8192;
uint16_t pc_g		= (br->instruction_addr >> 2) % 8192;
uint16_t pc_B		= pc_g & 0x3FF;

uint16_t X_indx = path_hist_value ^ pc_g;
uint16_t mux_sel= Select_prediction[pc_B];

bool conditional_check	= br->is_conditional;

bool prediction;

if(conditional_check)
{
	switch(mux_sel)
	{
		case 0:
		case 1:
			prediction = (Bimodal[pc_B] > 1);
			break;
		case 2:
		case 3:
			prediction = (Gshare[X_indx] > 1);
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
uint16_t path_hist_value  = GHR % 8192;
uint16_t pc_g		= (br->instruction_addr >> 2) % 8192;
uint16_t pc_B		= pc_g & 0x3FF;

bool conditional_check	  = br->is_conditional;
uint16_t X_indx = path_hist_value ^ pc_g;

bool local_pred 	  	  = (Bimodal[pc_B] > 1);
bool global_pred          = (Gshare[X_indx] > 1);
    
if(conditional_check)
{
	GHR = (GHR << 1) | taken;
	
	if (global_pred == taken && local_pred != taken)
	{
		Select_prediction[pc_B] += (Select_prediction[pc_B] < 3);
	} 
	else if (global_pred != taken && local_pred == taken)
	{
		Select_prediction[pc_B] -= (Select_prediction[pc_B] > 0);
	}
	
	if (taken)
	{
		Gshare[X_indx]  +=(Gshare[X_indx] < 3);
		Bimodal[pc_B]   +=(Bimodal[pc_B] < 3);
	} 
	else
	{
		Gshare[X_indx]  -=(Gshare[X_indx] > 0);	
		Bimodal[pc_B]   -=(Bimodal[pc_B] > 0);
	}
	}
else
{
	GHR = (GHR << 1) | taken;
}
}
