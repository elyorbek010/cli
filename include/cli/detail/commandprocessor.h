/*******************************************************************************
 * CLI - A simple command line interface.
 * Copyright (C) 2016-2021 Daniele Pallastrelli
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

#ifndef CLI_DETAIL_COMMANDPROCESSOR_H_
#define CLI_DETAIL_COMMANDPROCESSOR_H_

#include <functional>
#include <string>
#include "terminal.h"
#include "inputdevice.h"
#include "../cli.h" // CliSession
#include "commonprefix.h"

namespace cli
{
namespace detail
{

/**
 * @class CommandProcessor
 * @brief This class handles user input and processes commands in a CLI session.
 *
 * The CommandProcessor class is responsible for handling user input from an InputDevice,
 * processing the input into commands, and executing these commands in a CLI session.
 * It also provides functionality for command history navigation and command auto-completion.
 *
 * @tparam SCREEN The type of the terminal screen.
 */
template <typename SCREEN>
class CommandProcessor
{
public:
    /**
     * @brief Construct a new CommandProcessor object.
     *
     * @param _session The CLI session to be managed.
     * @param _kb The input device to be used.
     */
    CommandProcessor(CliSession& _session, InputDevice& _kb) :
        session(_session),
        terminal(session.OutStream()),
        kb(_kb)
    {
        kb.Register( [this](auto key){ this->Keypressed(key); } );
    }

private:

    /**
     * @brief Handle a keypress event.
     *
     * @param k The key that was pressed.
     */
    void Keypressed(std::pair<KeyType, char> k)
    {
        const std::pair<Symbol,std::string> s = terminal.Keypressed(k);
        NewCommand(s);
    }

    /**
     * @brief Process a new command.
     *
     * @param s The symbol and string representing the command.
     */
    void NewCommand(const std::pair<Symbol, std::string>& s)
    {
        switch (s.first)
        {
            case Symbol::nothing:
            {
                break;
            }
            case Symbol::eof:
            {
                session.Exit();
                break;
            }
            case Symbol::command:
            {
                kb.DeactivateInput();
                session.Feed(s.second);
                session.Prompt();
                kb.ActivateInput();
                break;
            }
            case Symbol::down:
            {
                terminal.SetLine(session.NextCmd());
                break;
            }
            case Symbol::up:
            {
                auto line = terminal.GetLine();
                terminal.SetLine(session.PreviousCmd(line));
                break;
            }
            case Symbol::tab:
            {
                auto line = terminal.GetLine();
                auto completions = session.GetCompletions(line);

                if (completions.empty())
                    break;
                if (completions.size() == 1)
                {
                    terminal.SetLine(completions[0]+' ');
                    break;
                }

                auto commonPrefix = CommonPrefix(completions);
                if (commonPrefix.size() > line.size())
                {
                    terminal.SetLine(commonPrefix);
                    break;
                }
                session.OutStream() << '\n';
                std::string items;
                std::for_each( completions.begin(), completions.end(), [&items](auto& cmd){ items += '\t' + cmd; } );
                session.OutStream() << items << '\n';
                session.Prompt();
                terminal.ResetCursor();
                terminal.SetLine( line );
                break;
            }
            case Symbol::clear:
            {
                const auto currentLine = terminal.GetLine();
                terminal.Clear();
                session.Prompt();
                terminal.ResetCursor();
                terminal.SetLine(currentLine);
                break;
            }
        }

    }

    CliSession& session;
    Terminal<SCREEN> terminal;
    InputDevice& kb;
};

} // namespace detail
} // namespace cli

#endif // CLI_DETAIL_COMMANDPROCESSOR_H_


