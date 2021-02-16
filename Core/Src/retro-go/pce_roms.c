
extern const rom_system_t pce_system;
extern const uint8_t _binary__Users_gary_game_watch_game_and_watch_retro_go_roms_pce_Dodge_Ball__Japan__pce_start[];
uint8_t SAVE_PCE_0[77824]  __attribute__((section (".saveflash"))) __attribute__((aligned(4096)));

const retro_emulator_file_t pce_roms[] = {
	{
		.name = "Dodge Ball (Japan)",
		.ext = "pce",
		.address = _binary__Users_gary_game_watch_game_and_watch_retro_go_roms_pce_Dodge_Ball__Japan__pce_start,
		.size = 262144,
		.save_address = SAVE_PCE_0,
		.save_size = sizeof(SAVE_PCE_0),
		.system = &pce_system,
		.region = REGION_NTSC,
	},

};
const uint32_t pce_roms_count = 1;

const rom_system_t pce_system = {
	.system_name = "PC Engine",
	.roms = pce_roms,
	.extension = "pce",
	.roms_count = 1,
};
