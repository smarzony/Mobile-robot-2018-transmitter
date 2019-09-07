void display_draw()
{


}
/*
void display_refresh_old()
{	

	int menuItem = (rotory_encoder.value % MENU_SIZE);

	// constant position elements
	display.setTextSize(1);
	display.setTextColor(WHITE);

	// display message counter to check if cpu isn't hanged
	display.setCursor(0, 55);
	display.print(message_counter/4);

	// display actual cursor position
	display.setCursor(115, 55);
	display.print(menuItem);

	// menu cursor - to be changed to inverse display
	display.setCursor(1, (menuItem * 10) );
	display.print('>');

	// moving around and changing menus
	if (digitalRead(ROTORY_ENCODER_PUSHBUTTON) == 0)
	{
		if (menu_no == 0)
		{
			menu_no = menuItem + 1;
		}
		else if (menu_no != 0)
		{
			if (menuItem == 4)
				menuItem = 1;
		}
	}


	// display menu from list
	for (int i = 0; i <= MENU_SIZE; i++)
	{
		display.setCursor(10, (i * 10));
		display.print(menu_item_list[menu_no][i]);
	}
	   
	display.display();
	display.clearDisplay();
} 
*/

void display_refresh()
{/*
	const int menu_size = 5;
	//int menu_level[menu_size][menu_size] = { 0, 0 };
	int menuItem = (rotory_encoder.value % menu_size) + 1;
	if (rotory_encoder.value > menu_size)
		rotory_encoder.value = 1;

	if (rotory_encoder.value < 1)
		rotory_encoder.value = menu_size;
*/
	// constant place elements
	display.setTextSize(1);
	display.setTextColor(WHITE);

	display.setCursor(0, 55);
	display.print(message_counter / 4);
/*
	display.setCursor(115, 55);
	display.print(menuItem);

	display.setCursor(1, ((menuItem - 1) * 10));
	display.print('>');



	// variable elements
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
*/

	display.display();
	display.clearDisplay();
}
