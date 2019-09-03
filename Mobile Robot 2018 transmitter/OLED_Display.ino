void display_draw()
{


}

void display_refresh()
{	
	int menuSize = 5;
	int menuItem = (rotory_encoder.value % menuSize) + 1;
	/*if (rotory_encoder.value > menuSize)
		rotory_encoder.value = 1;

	if (rotory_encoder.value < 1)
		rotory_encoder.value = menuSize;
*/


	display.setTextSize(1);
	display.setTextColor(WHITE);

	display.setCursor(0, 55);
	display.print(message_counter/4);

	display.setCursor(115, 55);
	display.print(menuItem);

	display.setCursor(1, ((menuItem - 1) * 10) );
	display.print('>');



	
	display.setCursor(10, 0);
	display.print("VL : ");
	display.println(message_receive.velocity_measured_left);
	
	display.setCursor(10, 10);
	display.print("VR : ");
	display.println(message_receive.velocity_measured_left);
	
	display.setCursor(10, 20);
	display.print("MSG: ");
	display.println(message_receive.message_no);
	
	display.setCursor(10, 30);
	display.print("DIS: ");
	display.println(message_receive.distance);
	
	display.setCursor(10, 40);
	display.print("MOD: ");

	String output;
	switch (message_receive.control_mode)
	{
		case CONTROLS_STANDARD:
			output = "Standard";
			break;

		case CONTROLS_ENCHANCED:
			output = "Enchanced";
			break;

		case CONTROLS_MEASURED:
			output = "Measured";
			break;
	}
	display.println(output);


	display.display();
	display.clearDisplay();
} 