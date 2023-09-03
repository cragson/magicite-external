#include <iostream>
#include <Windows.h>
#include <vector>

#include "globals.hpp"
#include "offsets.hpp"


std::uintptr_t find_pattern( const std::vector< uint8_t >& bytes, const std::wstring& pattern )
{
	// Old code, maybe I will rewrite it in the future

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
}

int main()
{
	SetConsoleTitleA( "osmium: magicite" );

	printf( "[>] Waiting for the game.." );

	while( !Globals::g_pProcess->setup_process( L"Magicite.exe" ) )
		Sleep( 420 );

	printf( "done!\n" );

	printf( "[>] Preparing the cheat.." );

	while( !Globals::g_pCheat->setup_offsets() )
		Sleep( 420 );

	if( Offsets::player_ptr < 0x1000000 )
	{
		printf(
			"\n\n[!] Failed to initialize player pointer properly, please start a game and start the cheat afterwards!\n"
		);

		Globals::g_pCheat->shutdown();

		return EXIT_FAILURE;
	}

	while( !Globals::g_pCheat->setup_features() )
		Sleep( 420 );

	printf( "done!\n" );

	Globals::g_pCheat->print_offsets();

	Globals::g_pCheat->print_features();

	printf( "Have fun! ;-)\n" );

	while( 0xDEAD != 0xAFFE )
	{
		if( GetAsyncKeyState( VK_END ) & 0x8000 )
			break;

		Globals::g_pCheat->run();


		Sleep( 1 );
	}

	Globals::g_pCheat->shutdown();

	printf( "Goodbye!\n" );

	Sleep( 3000 );

	return EXIT_SUCCESS;
}

/*
int main()
{
    const auto hwnd = FindWindowA(nullptr,"Magicite");

    DWORD pid = -1;

    GetWindowThreadProcessId(hwnd, &pid);

    const auto handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    printf("[+] window: 0x%p\n", hwnd);
    printf("[+] pid: %d\n", pid);
    printf("[+] handle: 0x%p\n\n", handle);

    std::uintptr_t page_base = {};
    
    std::vector< std::pair< std::uintptr_t, size_t > > pages;


    MEMORY_BASIC_INFORMATION mbi = {};
    RtlZeroMemory(&mbi, sizeof(mbi));

    while(VirtualQueryEx( handle, reinterpret_cast< LPCVOID >( page_base ), &mbi, sizeof(mbi) ) != 0)
    {
        const auto is_rwx = mbi.AllocationProtect & PAGE_EXECUTE_READWRITE;

        if(mbi.Type & MEM_PRIVATE && mbi.State & MEM_COMMIT && mbi.AllocationProtect & PAGE_EXECUTE_READWRITE && mbi.RegionSize == 0x00100000)
        {
            printf("[+] Found possible memory page: 0x%08X 0x%08X (%d bytes)\n", mbi.BaseAddress, mbi.AllocationBase, mbi.RegionSize);

            pages.emplace_back(reinterpret_cast<std::uintptr_t>(mbi.AllocationBase), mbi.RegionSize);
        }

        page_base += mbi.RegionSize;
    }

	std::vector< uint8_t > dump_page = {};

	for( const auto& page : pages )
	{
		dump_page.resize(page.second);

		ReadProcessMemory(handle, reinterpret_cast< LPCVOID >(page.first ), dump_page.data(), page.second, nullptr);

		const auto offset = find_pattern(dump_page, L"8B 05 ? ? ? ? 85 C0 7E 0B");

		if (offset)
		{
			printf("\n[+] Found pattern: 0x%08X\n", offset + page.first);

			std::uintptr_t player_ptr = {};

			memcpy(&player_ptr,&dump_page.at(offset + 2), sizeof(std::uintptr_t));

			printf("[+] Player offset: 0x%08X\n", player_ptr);
		}
			

		dump_page.clear();
	}

    CloseHandle(handle);

    return 0;
}

*/
