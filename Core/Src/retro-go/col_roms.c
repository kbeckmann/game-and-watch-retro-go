
extern const rom_system_t col_system;
extern const uint8_t _binary__home_ludo_hack_gw_homebrew_zx81zx81_repo_game_and_watch_retro_go_roms_col_cobra_col_start[];
uint8_t SAVE_COL_0[61440]  __attribute__((section (".saveflash"))) __attribute__((aligned(4096)));
extern const uint8_t _binary__home_ludo_hack_gw_homebrew_zx81zx81_repo_game_and_watch_retro_go_roms_col_dkjr_col_start[];
uint8_t SAVE_COL_1[61440]  __attribute__((section (".saveflash"))) __attribute__((aligned(4096)));
extern const uint8_t _binary__home_ludo_hack_gw_homebrew_zx81zx81_repo_game_and_watch_retro_go_roms_col_antar_col_start[];
uint8_t SAVE_COL_2[61440]  __attribute__((section (".saveflash"))) __attribute__((aligned(4096)));

const retro_emulator_file_t col_roms[] = {
	{
		.name = "cobra",
		.ext = "col",
		.address = _binary__home_ludo_hack_gw_homebrew_zx81zx81_repo_game_and_watch_retro_go_roms_col_cobra_col_start,
		.size = 8192,
		.save_address = SAVE_COL_0,
		.save_size = sizeof(SAVE_COL_0),
		.system = &col_system,
		.region = REGION_NTSC,
	},
	{
		.name = "dkjr",
		.ext = "col",
		.address = _binary__home_ludo_hack_gw_homebrew_zx81zx81_repo_game_and_watch_retro_go_roms_col_dkjr_col_start,
		.size = 32768,
		.save_address = SAVE_COL_1,
		.save_size = sizeof(SAVE_COL_1),
		.system = &col_system,
		.region = REGION_NTSC,
	},
	{
		.name = "antar",
		.ext = "col",
		.address = _binary__home_ludo_hack_gw_homebrew_zx81zx81_repo_game_and_watch_retro_go_roms_col_antar_col_start,
		.size = 16384,
		.save_address = SAVE_COL_2,
		.save_size = sizeof(SAVE_COL_2),
		.system = &col_system,
		.region = REGION_NTSC,
	},

};
const uint32_t col_roms_count = 3;

const rom_system_t col_system = {
	.system_name = "Colecovision",
	.roms = col_roms,
	.extension = "col",
	.roms_count = 3,
};
