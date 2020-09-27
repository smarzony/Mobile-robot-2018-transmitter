#include "Arduino.h"


//void rotoryEncoderHandler()
//{
//	rotory_encoder.clk_actual = digitalRead(ROTORY_ENCODER_CLK);
//	rotory_encoder.dt_actual = digitalRead(ROTORY_ENCODER_DT);
//
//	if (((rotory_encoder.clk_prev == 0 && rotory_encoder.clk_actual == 1 && rotory_encoder.dt_actual == 1) ||
//		(rotory_encoder.clk_prev == 1 && rotory_encoder.clk_actual == 0 && rotory_encoder.dt_actual == 0))
//		&& (now - last_encloder_change > ROTORY_ENCODER_CHANGE_MIN_TIME))
//	{
//		last_encloder_change = now;
//		rotory_encoder.value_int--;
//	}
//	if (((rotory_encoder.clk_prev == 1 && rotory_encoder.clk_actual == 0 && rotory_encoder.dt_actual == 1) ||
//		(rotory_encoder.clk_prev == 0 && rotory_encoder.clk_actual == 1 && rotory_encoder.dt_actual == 0))
//		&& (now - last_encloder_change > ROTORY_ENCODER_CHANGE_MIN_TIME))
//	{
//		last_encloder_change = now;
//		rotory_encoder.value_int++;
//	}
//
//	rotory_encoder.clk_prev = digitalRead(ROTORY_ENCODER_CLK);
//	rotory_encoder.dt_prev = digitalRead(ROTORY_ENCODER_DT);
//}

void rotoryEncoderHandler()
{
  #ifdef PROMINI
  rotory_encoder.clk_actual = digitalRead(ROTORY_ENCODER_CLK);
  rotory_encoder.dt_actual = digitalRead(ROTORY_ENCODER_DT);
  #endif

  #ifdef NODEMCU
  rotory_encoder.clk_actual = remoteIO.digitalRead(ROTORY_ENCODER_CLK);
  rotory_encoder.dt_actual = remoteIO.digitalRead(ROTORY_ENCODER_DT);
  #endif


    if (rotory_encoder.clk_actual == LOW && rotory_encoder.clk_prev == HIGH)
      {
        if (rotory_encoder.dt_actual == HIGH and now - last_encloder_change > ROTORY_ENCODER_CHANGE_MIN_TIME)
        {
          last_encloder_change = now;
          rotory_encoder.value_int--;
        }
        else if (rotory_encoder.dt_actual == LOW  and now - last_encloder_change > ROTORY_ENCODER_CHANGE_MIN_TIME)
          {
            last_encloder_change = now;
            rotory_encoder.value_int++;            
          }
      }

  rotory_encoder.clk_prev = rotory_encoder.clk_actual;
  rotory_encoder.dt_prev = rotory_encoder.dt_actual;
}
