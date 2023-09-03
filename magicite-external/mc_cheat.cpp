#include "mc_cheat.hpp"

#include "ft_godmode.hpp"
#include "ft_money.hpp"
#include "ft_speed.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "offsets.hpp"

bool mc_cheat::setup_features()
{
	auto god = std::make_unique< ft_godmode >();
	god->set_activation_delay( 420 );
	god->set_name( L"Godmode" );
	god->set_print_status( true );
	god->set_virtual_key_code( VK_NUMPAD1 );

	this->m_features.push_back( std::move( god ) );

	auto speed = std::make_unique< ft_speed >();
	speed->set_activation_delay( 420 );
	speed->set_name( L"Speedhack" );
	speed->set_print_status( true );
	speed->set_virtual_key_code( VK_NUMPAD2 );

	this->m_features.push_back( std::move( speed ) );

	auto money = std::make_unique< ft_money >();
	money->set_activation_delay( 420 );
	money->set_name( L"Add Money" );
	money->set_print_status( false );
	money->set_virtual_key_code( VK_NUMPAD3 );

	this->m_features.push_back( std::move( money ) );

	return true;
}

bool mc_cheat::setup_offsets()
{
	const auto game = Globals::g_pProcess->get_image_ptr_by_name( L"Magicite.exe" );

	if( !game )
		return false;

	const auto find_pattern = [&]( const std::vector< uint8_t >& bytes, const std::wstring& pattern )
	{
		std::vector< uint8_t > m_vecPattern;

		std::string Temp = std::string();

		std::string strPattern = std::string( pattern.begin(), pattern.end() );
		std::erase_if( strPattern, isspace );

		// Convert string pattern to byte pattern
		for( const auto& pattern_byte : strPattern )
		{
			if( pattern_byte == '?' )
			{
				m_vecPattern.emplace_back( WILDCARD_BYTE );
				continue;
			}

			if( Temp.length() != 2 )
				Temp += pattern_byte;

			if( Temp.length() == 2 )
			{
				std::erase_if( Temp, isspace );
				auto converted_pattern_byte = strtol( Temp.c_str(), nullptr, 16 ) & 0xFFF;
				m_vecPattern.emplace_back( converted_pattern_byte );
				Temp.clear();
			}
		}
		const auto vector_size = m_vecPattern.size();

		// m_vecPattern contains now the converted byte pattern
		// Search now the memory area

		bool           found      = false;
		std::uintptr_t found_addr = 0;

		for( std::uintptr_t current_addr = 0; current_addr < bytes.size(); current_addr++ )
		{
			if( found )
				break;

			for( uint8_t i = 0; i < vector_size; i++ )
			{
				const auto current_byte = bytes.at( current_addr + i );

				const auto pattern_byte = m_vecPattern.at( i );

				if( static_cast< uint8_t >( pattern_byte ) == WILDCARD_BYTE )
				{
					if( i == vector_size - 1 )
					{
						found      = true;
						found_addr = current_addr;
						break;
					}
					continue;
				}

				if( static_cast< uint8_t >( current_byte ) != pattern_byte )
					break;

				if( i == vector_size - 1 )
				{
					found_addr = current_addr;
					found      = true;
				}
			}
		}

		return found_addr;
	};

	std::uintptr_t page_base = {};

	std::vector< std::pair< std::uintptr_t, size_t > > pages;


	MEMORY_BASIC_INFORMATION mbi = {};
	RtlZeroMemory( &mbi, sizeof(mbi) );

	while( VirtualQueryEx( Globals::g_pProcess->get_process_handle(), reinterpret_cast< LPCVOID >( page_base ), &mbi,
	                       sizeof( mbi )
	) != 0 )
	{
		if( mbi.Type & MEM_PRIVATE && mbi.State & MEM_COMMIT && mbi.AllocationProtect & PAGE_EXECUTE_READWRITE && mbi.
			RegionSize == 0x00100000 )
			pages.emplace_back( reinterpret_cast< std::uintptr_t >( mbi.AllocationBase ), mbi.RegionSize );

		page_base += mbi.RegionSize;
	}

	std::vector< uint8_t > dump_page = {};

	for( const auto& [ page_addr, page_size ] : pages )
	{
		dump_page.resize( page_size );

		ReadProcessMemory( Globals::g_pProcess->get_process_handle(), reinterpret_cast< LPCVOID >( page_addr ),
		                   dump_page.data(), page_size, nullptr
		);

		if( const auto offset = find_pattern( dump_page, L"8B 05 ? ? ? ? 85 C0 7E 0B" ); offset )
		{
			Offsets::godmode_patch = page_addr + offset;

			std::uintptr_t pPlayer = {};

			memcpy( &pPlayer, &dump_page.at( offset + 2 ), sizeof( std::uintptr_t ) );

			Offsets::player_ptr = pPlayer - 0x30;
		}

		dump_page.clear();
	}


	return true;
}

void mc_cheat::print_features()
{
	printf( "\n" );

	printf( "Feature-Name -> Feature-Hotkey\n" );

	for( const auto& feature : this->m_features )
		printf( "[>] %-25ws -> %s\n", feature->get_name().c_str(),
		        utils::virtual_key_as_string( feature->get_virtual_key_code() ).c_str()
		);

	printf( "\n" );
}

void mc_cheat::print_offsets()
{
	printf( "\n" );

	const auto msg = []( const std::string& name, const std::uintptr_t value )
	{
		printf( "[>] %-35s -> 0x%08X\n", name.c_str(), value );
	};

	msg( "Godmode Patch", Offsets::godmode_patch );
	msg( "Player", Offsets::player_ptr );


	printf( "\n" );
}

void mc_cheat::run()
{
	for( const auto& feature : this->m_features )
	{
		// before tick'ing the feature, check first if the state will eventually change
		if( GetAsyncKeyState( feature->get_virtual_key_code() ) & 0x8000 )
			feature->toggle();

		// let the feature tick() when active
		if( feature->is_active() )
			feature->tick();
	}
}

void mc_cheat::shutdown()
{
	// disable every feature here
	for( const auto& feature : this->m_features )
		if( feature->is_active() )
			feature->disable();

	// clear image map here
	if( Globals::g_pProcess )
		Globals::g_pProcess->clear_image_map();
}
