#include "ft_money.hpp"

#include "globals.hpp"
#include "offsets.hpp"
#include "structs.hpp"

void ft_money::on_enable()
{
	Globals::g_pProcess->write< int32_t >( Offsets::player_ptr + offsetof( playerinfo, m_iMoney ), 13337 );

	printf( "[#] Added money!\n" );

	this->disable();
}
