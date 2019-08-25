
void rotoryEncoderHandler()
{
	rotory_encoder.clk_actual = digitalRead(ROTORY_ENCODER_CLK);
	rotory_encoder.dt_actual = digitalRead(ROTORY_ENCODER_DT);

	if ((rotory_encoder.clk_prev == 0 && rotory_encoder.clk_actual == 1 && rotory_encoder.dt_actual == 1) ||
		(rotory_encoder.clk_prev == 1 && rotory_encoder.clk_actual == 0 && rotory_encoder.dt_actual == 0)
		&& (now - last_encloder_change > ROTORY_ENCODER_CHANGE_MIN_TIME))
	{
		last_encloder_change = now;
		rotory_encoder.value--;
	}
	if ((rotory_encoder.clk_prev == 1 && rotory_encoder.clk_actual == 0 && rotory_encoder.dt_actual == 1) ||
		(rotory_encoder.clk_prev == 0 && rotory_encoder.clk_actual == 1 && rotory_encoder.dt_actual == 0)
		&& (now - last_encloder_change > ROTORY_ENCODER_CHANGE_MIN_TIME))
	{
		last_encloder_change = now;
		rotory_encoder.value++;
	}

	rotory_encoder.clk_prev = digitalRead(ROTORY_ENCODER_CLK);
	rotory_encoder.dt_prev = digitalRead(ROTORY_ENCODER_DT);
}


