#include "ft_speed.hpp"

#include "globals.hpp"
#include "offsets.hpp"
#include "structs.hpp"

void ft_speed::tick()
{
	Globals::g_pProcess->write< float >( Offsets::player_ptr + offsetof( playerinfo, m_flSpeed ), 32.69f );
}
