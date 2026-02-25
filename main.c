#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "C Baccarat"
#define TEXT_PADDING 10
#define TITLE_FONT_SIZE 48
#define TEXT_FONT_SIZE 24
#define SUIT_SIZE 13
#define DECK_SIZE 52
#define DEALT_CARDS 4 // 3 + 1, last place for null byte
#define BET_AMOUNTS_SIZE 10
#define BUTTON_WIDTH 100.0
#define BUTTON_HEIGHT 60.0
#define LABEL_SIZE 128
#define DIVISOR 100 // magic number to convert from pennies to dollars

typedef struct {
	char name[LABEL_SIZE];
	int x_pos, y_pos;
	int text_font_size;
	Color text_color;
}Label;

typedef long int i64;
typedef unsigned char u8;
typedef unsigned short u16;

typedef Rectangle Button;

typedef enum {
	PLAYER=0,
	BANKER=1,
	TIE=2,
	NONE=3 // user has to choose every time they place a bet
}BetState;

struct GameState {
	char player_cards[DEALT_CARDS]; // 3 + 1, last space is for null byte
	int player_cards_drawn;
	char banker_cards[DEALT_CARDS];
	int banker_cards_drawn;
	u16 money_bet; // amount of money user bet
	BetState bet; // who the user decided to bet on
	BetState winner; // who won round
	i64 money; // total amount of user money
};

void init_gamestate(struct GameState* gs, i64 init_money) {

	memset(gs->player_cards, '\0', DEALT_CARDS);
	memset(gs->banker_cards, '\0', DEALT_CARDS);

	gs->bet = NONE;
	gs->winner = NONE;

	gs->money = init_money;
	gs->money_bet = 0;

	gs->player_cards_drawn = 0;
	gs->banker_cards_drawn = 0;
}

void print_gamestate(struct GameState* gs) { // debug function
	printf("\nGAMESTATE\nplayer cards: %s\tbanker cards: %s\tmoney bet: %d\tbet: %d\twinner: %d\tmoney: %ld\n",
		gs->player_cards, gs->banker_cards, gs->money_bet, gs->bet, gs->winner, gs->money);
}

void shuffle(const char* deck, char* shuffled_deck) {
	char tmp[DECK_SIZE];

	for (int i = 0; i < DECK_SIZE; i++) {
		tmp[i] = deck[rand() % DECK_SIZE];
	}

	strncpy(shuffled_deck, tmp, DECK_SIZE);
}

void set_label(Label* l, char* name, int x_pos, int y_pos, int text_font_size, Color text_color) {
	l->name[LABEL_SIZE - 1] = '\0'; // last character will always be NULL
	
	strncpy(l->name, name, LABEL_SIZE - 2);

	l->x_pos = x_pos;
	l->y_pos = y_pos;
	l->text_font_size = text_font_size;
	l->text_color = text_color;
}


u8 card_value(char card) {
	switch (card) {
		case 'A':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'T':
			return 0;
		case 'J':
			return 0;
		case 'Q':
			return 0;
		case 'K':
			return 0;
	}
}

char* card_str(char card) {
	switch (card) {
		case 'A':
			return "A\0";
		case '2':
			return "2\0";
		case '3':
			return "3\0";
		case '4':
			return "4\0";
		case '5':
			return "5\0";
		case '6':
			return "6\0";
		case '7':
			return "7\0";
		case '8':
			return "8\0";
		case '9':
			return "9\0";
		case 'T':
			return "10\0";
		case 'J':
			return "J\0";
		case 'Q':
			return "Q\0";
		case 'K':
			return "K\0";
	}
}

u8 sum_cards(u8 (*fp)(char), int start, int stop, char* deck_ptr) {
	u8 sum = 0;
	for (int i = start; i < stop; i++) {
		sum += fp(deck_ptr[i]);
	}

	return sum;
}

void init_bets(u16* bet_amounts) {
	for (int i = 0; i < BET_AMOUNTS_SIZE; i++) {
		bet_amounts[i] = (i+1) * DIVISOR;
	}
}

void init_bet_buttons(Button* bet_buttons) {
	// set x and y positions on buttons, init bets_amount array
	float button_y = (float)(WINDOW_HEIGHT/2);
	float button_x = 0.0f;

	for (int i = 0; i < BET_AMOUNTS_SIZE; i++) { // bet buttons

		if (button_x > WINDOW_WIDTH || (button_x + BUTTON_WIDTH) > WINDOW_WIDTH) {
			button_x = 0.0f;
			button_y += BUTTON_HEIGHT + 3;
		}
		bet_buttons[i] = (Button){button_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT};
		button_x += BUTTON_WIDTH + 3;
	}

}

void place_bet(struct GameState* gs, u16 bet_amount) {
	gs->money_bet = bet_amount;
	gs->money -= bet_amount;
}

int main(void) {
	struct GameState gs;
	int player_drew_three; // flag to detect if player drew three cards
	Button player_select;
	Button banker_select;
	Button tie_select;
	int display_cards = 0; // TODO: temp way of displaying cards
	Vector2 mouse_pos; // stores mouse position for detecting button click
	u16 bet_amounts[BET_AMOUNTS_SIZE];
	Button bet_buttons[BET_AMOUNTS_SIZE];
	Color button_color = WHITE;


	srand(time(0)); // seed random number generator with current system time
	char tmp[16] = {'\0'}; // tmp var to draw bet amounts on buttons

	const char deck[DECK_SIZE] = { // T for 10 to keep everything one character
		'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K',
		'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K',
		'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K',
		'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'
	};

	char shuffled_deck[DECK_SIZE];

	const char tableau[9][10] = { // [banker][player]
		'H','H','H','H','H','H','H','H','H','H',
		'H','H','H','H','H','H','H','H','H','H',
		'H','H','H','H','H','H','H','H','H','H',
		'H','H','H','H','H','H','H','H','S','H',
		'S','S','H','H','H','H','H','H','S','S',
		'S','S','S','S','H','H','H','H','S','S',
		'H','H','H','H','H','H','H','H','H','H',
		'S','S','S','S','S','S','H','H','S','S',
		'S','S','S','S','S','S','S','S','S','S',
	};

	// init structs and arrays
	init_gamestate(&gs, 20*DIVISOR);
	init_bets(bet_amounts);
	init_bet_buttons(bet_buttons);


	// player sum
	u8 player_sum;

	//banker sum
	u8 banker_sum;

	// labels
	Label title_label;
	Label player_label;
	Label banker_label;
	Label tie_label;

	// setup labels
	set_label(&title_label, WINDOW_TITLE, strlen(WINDOW_TITLE) + (WINDOW_WIDTH/4), 0, TITLE_FONT_SIZE, BLACK);
	set_label(&player_label, "", TEXT_PADDING, TEXT_PADDING + TITLE_FONT_SIZE, TEXT_FONT_SIZE, BLACK);
	set_label(&banker_label, "Banker", WINDOW_WIDTH + TEXT_PADDING - (TEXT_FONT_SIZE * strlen("banker")), TEXT_PADDING + TITLE_FONT_SIZE, TEXT_FONT_SIZE, BLACK);
	set_label(&tie_label, "Tie", WINDOW_WIDTH / 2 - (TEXT_FONT_SIZE * strlen("Tie")), TEXT_PADDING + TITLE_FONT_SIZE, TEXT_FONT_SIZE, BLACK);

	
	sprintf(player_label.name, "Player $%ld", gs.money/DIVISOR); // insert money var to player string

	
	// select buttons
	player_select = (Button){player_label.x_pos, player_label.y_pos, strlen(player_label.name)*TEXT_FONT_SIZE-100, TEXT_FONT_SIZE};
	banker_select = (Button){banker_label.x_pos, banker_label.y_pos, strlen(banker_label.name)*TEXT_FONT_SIZE-50, TEXT_FONT_SIZE};
	tie_select = (Button){tie_label.x_pos, tie_label.y_pos, strlen(tie_label.name)*TEXT_FONT_SIZE-30, TEXT_FONT_SIZE};

	// init raylib window
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		// update
		mouse_pos = GetMousePosition();

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			// check if user selects PLAYER, BANKER, OR TIE
			if (CheckCollisionPointRec(mouse_pos, player_select)) {
				gs.bet = PLAYER;
			}
			if (CheckCollisionPointRec(mouse_pos, banker_select)) {
				gs.bet = BANKER;
			}
			if (CheckCollisionPointRec(mouse_pos, tie_select)) {
				gs.bet = TIE;
			}

			for (int i = 0; i < BET_AMOUNTS_SIZE; i++)  {
				if (CheckCollisionPointRec(mouse_pos, bet_buttons[i]) && gs.bet != NONE) {
					if (gs.money - bet_amounts[i] < 0) {
						goto end3;
					}

					display_cards = 1;
					place_bet(&gs, bet_amounts[i]);

					shuffle(deck, shuffled_deck);

					strncpy(gs.player_cards, shuffled_deck, DEALT_CARDS - 1);
					strncpy(gs.banker_cards, shuffled_deck+3, DEALT_CARDS - 1);

					player_sum = sum_cards(&card_value, 0, 2, gs.player_cards) % 10;
					banker_sum = sum_cards(&card_value, 0, 2, gs.banker_cards) % 10;

					gs.player_cards_drawn = 2;
					gs.banker_cards_drawn = 2;

					if (player_sum <= 5) {
						player_sum = sum_cards(&card_value, 0, 3, gs.player_cards) % 10;
						gs.player_cards_drawn = 3;
						if (tableau[banker_sum][player_sum] == 'H') {
							banker_sum = sum_cards(&card_value, 0, 3, gs.banker_cards) % 10;
							gs.banker_cards_drawn = 3;
							goto end;
						}
					}

					if (banker_sum <= 5) {
						banker_sum = sum_cards(&card_value, 0, 3, gs.banker_cards) % 10;
						gs.banker_cards_drawn = 3;
					}


				end: 
					if (player_sum > banker_sum) {
						gs.winner = PLAYER;
					}
					else if (player_sum < banker_sum) {
						gs.winner = BANKER;
					}
					else {
						gs.winner = TIE;
					}

					if (gs.winner == TIE && gs.bet != TIE) {
						gs.money += gs.money_bet;
						goto end2;
					}

					if (gs.bet == gs.winner) {
						if (gs.winner == TIE) {
							gs.money_bet *= 8;
							gs.money += gs.money_bet;
							goto end2;
						}

						gs.money_bet *= 2;
						gs.money += gs.money_bet;
					}
				end2:
					gs.bet = NONE;
					gs.money_bet = 0;
					sprintf(player_label.name, "Player $%ld", gs.money/DIVISOR); // update player money

				end3:
					break;
				}
			}
		}


		// drawing

		BeginDrawing();
		ClearBackground(RAYWHITE);

		// draw UI text
		DrawText(title_label.name, title_label.x_pos, title_label.y_pos, title_label.text_font_size, title_label.text_color);
		DrawText(player_label.name, player_label.x_pos, player_label.y_pos, player_label.text_font_size, player_label.text_color);
		DrawText(banker_label.name, banker_label.x_pos, banker_label.y_pos, banker_label.text_font_size, banker_label.text_color);
		DrawText(tie_label.name, tie_label.x_pos, tie_label.y_pos, tie_label.text_font_size, tie_label.text_color);

		// draw cards and sums
		if (display_cards) {
			int offset = 0;
			// draw player cards

			//printf("Player Cards: %s\n", gs.player_cards);
			//strncpy(tmp, gs.player_cards, gs.player_cards_drawn);
			//DrawText(tmp, player_label.x_pos, player_label.y_pos + TEXT_PADDING * 3, TEXT_FONT_SIZE, BLACK);
			
			for (int i = 0; i < gs.player_cards_drawn; i++) {
				DrawText(card_str(gs.player_cards[i]), player_label.x_pos + (i*2) * TEXT_FONT_SIZE, player_label.y_pos + TEXT_PADDING * 3, TEXT_FONT_SIZE, BLACK);
			}
			

			// draw banker cards
			//printf("Banker Cards: %s\n", gs.banker_cards);
			//strncpy(tmp, gs.banker_cards, gs.banker_cards_drawn);
			//DrawText(tmp, banker_label.x_pos, banker_label.y_pos + TEXT_PADDING * 3, TEXT_FONT_SIZE, BLACK);
		
			for (int i = 0; i < gs.banker_cards_drawn; i++) {
				DrawText(card_str(gs.banker_cards[i]), banker_label.x_pos + offset, banker_label.y_pos + TEXT_PADDING * 3, TEXT_FONT_SIZE, BLACK);
				offset += TEXT_FONT_SIZE * 2;
			}
			

			// player sum to text
			sprintf(tmp, "%d", player_sum);
			DrawText(tmp, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3, TITLE_FONT_SIZE, BLACK);

			// banker sum to text
			sprintf(tmp, "%d", banker_sum);
			DrawText(tmp, WINDOW_WIDTH / 3 + TITLE_FONT_SIZE, WINDOW_HEIGHT / 3, TITLE_FONT_SIZE, BLACK);

		}

		// draw buttons
		for (int i = 0; i < BET_AMOUNTS_SIZE; i++) {
			if (i % 2 == 0) {
				button_color = GREEN;
			}
			else {
				button_color = BLUE;
			}
			if (gs.money < bet_amounts[i]) {
				button_color = GRAY;
			}
			DrawRectangleRec(bet_buttons[i], button_color);
			sprintf(tmp, "$%d", bet_amounts[i]/DIVISOR);
			DrawText(tmp, bet_buttons[i].x + (BUTTON_WIDTH/2) - (TEXT_FONT_SIZE/2), bet_buttons[i].y + (BUTTON_HEIGHT/2) - (TEXT_FONT_SIZE/2), TEXT_FONT_SIZE, BLACK);
		}


		// Draw select buttons
		switch(gs.bet) {
			case PLAYER:
				DrawRectangleLinesEx(player_select, 3.0f, GOLD);
				break;
			case BANKER:
				DrawRectangleLinesEx(banker_select, 3.0f, GOLD);
				break;
			case TIE:
				DrawRectangleLinesEx(tie_select, 3.0f, GOLD);
				break;
			case NONE:
				break;
			default:
				break;
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
