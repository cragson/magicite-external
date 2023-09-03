#include "ft_godmode.hpp"

#include "globals.hpp"
#include "offsets.hpp"

void ft_godmode::on_enable()
{
	Globals::g_pProcess->nop_bytes( Offsets::godmode_patch, 6 );
}

void ft_godmode::on_disable()
{
	Globals::g_pProcess->write_to_protected_memory< uint16_t >( Offsets::godmode_patch, 0x058B );
	Globals::g_pProcess->write_to_protected_memory< std::uintptr_t >( Offsets::godmode_patch + 2,
	                                                                  Offsets::player_ptr + 0x30
	);
}
