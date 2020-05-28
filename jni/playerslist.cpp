/*

SGBTEAM Player TAB By:FR


*/

#include "main.h"
#include "game/game.h"
#include "net/netgame.h"
#include "game/common.h"
#include "gui/gui.h"
#include "playerslist.h"
#include "vendor/imgui/imgui_internal.h"
#include "timer.hpp"
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>

extern CGUI *pGUI;
extern CGame *pGame;
extern CNetGame *pNetGame;

CPlayersList::CPlayersList()
{
    m_bIsActive = false;
    szInfo = " ";
    playersCount = 0;
    out = false;
}

CPlayersList::~CPlayersList()
{

}

void CPlayersList::Show(bool bShow)
{
    m_bIsActive = bShow;
    if(bShow != false) out = false;
}

const char MenandaiWarnaMulai = '{';
const char MenandaiWarnaSelesai = '}';

bool ProcessInlineHexColor( const char* start, const char* end, ImVec4& color )
{
    const int hexCount = ( int )( end - start );
    if( hexCount == 6 || hexCount == 8 )
    {
        char hex[9];
        strncpy( hex, start, hexCount );
        hex[hexCount] = 0;

        unsigned int hexColor = 0;
        if( sscanf( hex, "%x", &hexColor ) > 0 )
        {
            color.x = static_cast<float>((hexColor & 0xFF000000) >> 24) / 255.0f;
            color.y = static_cast<float>((hexColor & 0x00FF0000) >> 16) / 255.0f;
            color.z = static_cast<float>((hexColor & 0x0000FF00) >> 8) / 255.0f;

            if( hexCount == 8 )
            {
                 color.w = static_cast< float >( ( hexColor & 0xFF000000 ) >> 24 ) / 255.0f;
            }
            else
            {
                color.w = 1.0f;
            }

            return true;
        }
    }

    return false;
}

void TextDenganWarna( const char* fmt, ... )
{
    char AsuStr[4096];

    va_list argPtr;
    va_start( argPtr, fmt );
    vsnprintf( AsuStr, sizeof( AsuStr ), fmt, argPtr );
    va_end( argPtr );
    AsuStr[sizeof( AsuStr ) - 1] = '\0';

    bool pushedGayaWarna = false;
    const char* TextMulai = AsuStr;
    const char* TextnyaCur = AsuStr;
    while( TextnyaCur < ( AsuStr + sizeof( AsuStr ) ) && *TextnyaCur != '\0' )
    {
        if( *TextnyaCur == MenandaiWarnaMulai )
        {
            //print text
            if( TextnyaCur != TextMulai )
            {
                ImGui::TextUnformatted( TextMulai, TextnyaCur );
                ImGui::SameLine( 0.0f, 0.0f );
            }

            // Process color codenya
            const char* colorStart = TextnyaCur + 1;
            do
            {
                ++TextnyaCur;
            }
            while( *TextnyaCur != '\0' && *TextnyaCur != MenandaiWarnaSelesai );

            // Ganti warna
            if( pushedGayaWarna )
            {
                ImGui::PopStyleColor();
                pushedGayaWarna = false;
            }

            ImVec4 textColor;
            if( ProcessInlineHexColor( colorStart, TextnyaCur, textColor ) )
            {
                ImGui::PushStyleColor( ImGuiCol_Text, textColor );
                pushedGayaWarna = true;
            }

            TextMulai = TextnyaCur + 1;
        }
        else if( *TextnyaCur == '\n' )
        {
            // akumulasi print text dan melanjutkan ke selanjutnya
            ImGui::TextUnformatted( TextMulai, TextnyaCur );
            TextMulai = TextnyaCur + 1;
        }

        ++TextnyaCur;
    }

    if( TextnyaCur != TextMulai )
    {
        ImGui::TextUnformatted( TextMulai, TextnyaCur );
    }
    else
    {
        ImGui::NewLine();
    }

    if( pushedGayaWarna )
    {
        ImGui::PopStyleColor();
    }
}

void CPlayersList::Clear()
{
    m_bIsActive = false;
    szInfo = " ";
    playersCount = 0;
    out = false;
}

int lastPlayerCount = 0;
void CPlayersList::Render()
{
    if(!m_bIsActive) return;

    CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

    ImGuiIO &io = ImGui::GetIO();

    ImGui::StyleColorsClassic();
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(pGUI->ScaleX(8 * 2), pGUI->ScaleY(8 * 2)));

    char motd[256];

    if(pNetGame) sprintf(motd, "%.64s                                                   Player: %i", pNetGame->m_szHostName, playersCount + 1);

    ImGui::Begin(motd, nullptr, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);


    ImGui::ItemSize( ImVec2(0, 5) );


    ImGui::BeginChild("playersChild", ImVec2(700, 450), true, 
                ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if(pPlayerPool)
    {
        char colorp[10];
        char fixcolor[6];
        uint32_t colorplayer = pPlayerPool->GetLocalPlayer()->GetPlayerColor();
        uint32_t colorplayer2 = colorplayer;
        sprintf(colorp, "%06x", colorplayer2);
        sprintf(fixcolor, "%s", colorp);
        char header[5][550];
        ImGui::Columns(4, "Bar");
        sprintf(header[0], "id");
        ImGui::Text("%s", header[0]); ImGui::NextColumn();
        sprintf(header[1], "name");
        ImGui::Text("%s", header[1]); ImGui::NextColumn();
        sprintf(header[2], "score");
        ImGui::Text("%s", header[2]); ImGui::NextColumn();
        sprintf(header[3], "ping");
        ImGui::Text("%s", header[3]); ImGui::NextColumn();
        ImGui::Text("%d", pPlayerPool->GetLocalPlayerID());
        ImGui::NextColumn();
        //ImGui::Text("%s", fixcolor);
        TextDenganWarna( "{%s}%s", fixcolor, pPlayerPool->GetLocalPlayerName());
        ImGui::NextColumn();
        ImGui::Text("%d", pPlayerPool->GetLocalPlayerScore());
        ImGui::NextColumn();
        ImGui::Text("%d", pPlayerPool->GetLocalPlayerPing());
        ImGui::NextColumn();
    }



    for(uint32_t playerId = 0; playerId <= playersCount; playerId++)
    {
        if(pPlayerPool->GetSlotState(playerId) == true)
        {
            char color[1000][10];
            char colcol[1000][6];
            CRemotePlayer* pPlayer = pPlayerPool->GetAt(playerId);
            uint32_t dwColor = pPlayer->GetPlayerColor();
            uint32_t dwcolor2 = dwColor;
            char* pName = (char*)pPlayerPool->GetPlayerName(playerId);
            int pscore = (int)pPlayerPool->GetPlayerScore(playerId);
            sprintf(color[playerId], "%06x", dwcolor2);
            sprintf(colcol[playerId], "%s", color[playerId]);
            int pping = (int)pPlayerPool->GetPlayerPing(playerId);
            ImGui::Text("%i", playerId);
            ImGui::NextColumn();
            TextDenganWarna( "{%s}%s", colcol[playerId], pName);
            ImGui::NextColumn();
            ImGui::Text("%d", pscore);
            ImGui::NextColumn();
            ImGui::Text("%d", pping);
            ImGui::NextColumn();
        }
    }

    ImGui::EndChild();

    ImGui::SetWindowSize(ImVec2(-1, -1));
    ImVec2 size = ImGui::GetWindowSize();
    ImGui::SetWindowPos( ImVec2( ((io.DisplaySize.x - size.x)/2), ((io.DisplaySize.y - size.y)/2) ) );
    ImGui::End();
    ImGui::PopStyleVar();
}