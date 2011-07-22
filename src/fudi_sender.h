/*
 * This file is part of Spatosc.
 *
 * Copyright (c) 2010 Society for Arts and Technologies <info@sat.qc.ca>
 *
 * Spatosc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Spatosc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Spatosc.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * The FudiSender class
 */
#ifndef __FUDI_SENDER_H__
#define __FUDI_SENDER_H__

#include <string>

namespace inoui
{

/**
 * Sends FUDI messages to Pure Data.
 */
class FudiSender
{
    public:
        /**
         * @param host An IP address or "localhost"
         * @param port A port number.
         * @param isTcp Set to true to use TCP or false to use UDP.
         */
        FudiSender(std::string host, unsigned int port, bool isTcp);
        /**
         * Sends a FUDI message.
         * Note than FUDI messages always end with a semi-colon. (";")
         * When using the UDP protocol, they must also end with a newline character. ("\n")
         *
         * FUDI messages are made of ASCII strings whose atoms are separated by a space.
         * Atoms can be strings of string-formatted numbers.
         */
        bool sendFudi(const std::string &message);
    private:
        std::string host_;
        unsigned int port_;
        int protocol_;
        void sockerror(char *s);
        void x_closesocket(int fd);
};

} // end of namespace spatosc

#endif // __FUDI_SENDER_H__

