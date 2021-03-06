/*
 * Copyright (c) 2011-2013 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <boost/test/unit_test.hpp>
#include <bitcoin/bitcoin.hpp>

using namespace bc;

BOOST_AUTO_TEST_SUITE(uri_tests)

BOOST_AUTO_TEST_CASE(uri_parse_test)
{
    // Typical-looking URI:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse(
        "bitcoin:113Pfw4sFqN1T5kXUnKbqZHMJHN9oyjtgD?amount=0.1", result));
    BOOST_REQUIRE(result.address && result.address.get().encoded() ==
        "113Pfw4sFqN1T5kXUnKbqZHMJHN9oyjtgD");
    BOOST_REQUIRE(result.amount && result.amount.get() == 10000000);
    BOOST_REQUIRE(!result.label);
    BOOST_REQUIRE(!result.message);
    BOOST_REQUIRE(!result.r);
}

BOOST_AUTO_TEST_CASE(uri_parse_format_test)
{
    uri_parse_result result;

    // Various scheme spellings and blank structure elements:
    BOOST_REQUIRE( uri_parse("bitcoin:", result));
    BOOST_REQUIRE(!uri_parse("bitcorn:", result));
    BOOST_REQUIRE( uri_parse("BITCOIN:?", result));
    BOOST_REQUIRE( uri_parse("Bitcoin:?&", result));
    BOOST_REQUIRE(!uri_parse("bitcOin:&", result));

    // Various blank parameter elements:
    BOOST_REQUIRE( uri_parse("bitcoin:?x=y", result));
    BOOST_REQUIRE( uri_parse("bitcoin:?x=", result));
    BOOST_REQUIRE(!uri_parse("bitcoin:?=y", result));
    BOOST_REQUIRE(!uri_parse("bitcoin:?=", result));
    BOOST_REQUIRE( uri_parse("bitcoin:?x", result));
}

BOOST_AUTO_TEST_CASE(uri_parse_address_test)
{
    // Address only:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse(
        "bitcoin:113Pfw4sFqN1T5kXUnKbqZHMJHN9oyjtgD", result));
    BOOST_REQUIRE(result.address && result.address.get().encoded() ==
        "113Pfw4sFqN1T5kXUnKbqZHMJHN9oyjtgD");
    BOOST_REQUIRE(!result.amount);
    BOOST_REQUIRE(!result.label);
    BOOST_REQUIRE(!result.message);
    BOOST_REQUIRE(!result.r);
}

BOOST_AUTO_TEST_CASE(uri_parse_address_format_test)
{
    // Percent-encoding in address:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse(
        "bitcoin:%3113Pfw4sFqN1T5kXUnKbqZHMJHN9oyjtgD", result));
    BOOST_REQUIRE(result.address && result.address.get().encoded() ==
        "113Pfw4sFqN1T5kXUnKbqZHMJHN9oyjtgD");

    // Malformed addresses:
    BOOST_REQUIRE(!uri_parse("bitcoin:19l88", result));
    BOOST_REQUIRE(!uri_parse("bitcoin:19z88", result));
}

BOOST_AUTO_TEST_CASE(uri_parse_amount_test)
{
    // Amount only:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse("bitcoin:?amount=4.2", result));
    BOOST_REQUIRE(!result.address);
    BOOST_REQUIRE(result.amount && result.amount.get() == 420000000);
    BOOST_REQUIRE(!result.label);
    BOOST_REQUIRE(!result.message);
    BOOST_REQUIRE(!result.r);
}

BOOST_AUTO_TEST_CASE(uri_parse_amount_format_test)
{
    // Minimal amount:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse("bitcoin:?amount=.", result));
    BOOST_REQUIRE(result.amount && result.amount.get() == 0);

    // Malformed amounts:
    BOOST_REQUIRE(!uri_parse("bitcoin:amount=4.2.1", result));
    BOOST_REQUIRE(!uri_parse("bitcoin:amount=bob", result));
}

BOOST_AUTO_TEST_CASE(uri_parse_label_test)
{
    // Label only:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse("bitcoin:?label=test", result));
    BOOST_REQUIRE(!result.address);
    BOOST_REQUIRE(!result.amount);
    BOOST_REQUIRE(result.label && result.label.get() == "test");
    BOOST_REQUIRE(!result.message);
    BOOST_REQUIRE(!result.r);
}

BOOST_AUTO_TEST_CASE(uri_parse_escape_test)
{
    // Reserved symbol encoding and lowercase percent encoding:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse("bitcoin:?label=%26%3d%6b", result));
    BOOST_REQUIRE(result.label && result.label.get() == "&=k");

    // Malformed percent encoding:
    BOOST_REQUIRE(!uri_parse("bitcoin:label=%3", result));
    BOOST_REQUIRE(!uri_parse("bitcoin:label=%3G", result));
}

BOOST_AUTO_TEST_CASE(uri_parse_escape_utf8_test)
{
    // UTF-8 percent encoding:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse("bitcoin:?label=%E3%83%95", result));
    BOOST_REQUIRE(result.label && result.label.get() == "???");
}

BOOST_AUTO_TEST_CASE(uri_parse_strict_test)
{
    // Lenient parsing:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse(
        "bitcoin:?label=Some ?????????", result, false));
    BOOST_REQUIRE(result.label && result.label.get() == "Some ?????????");

    // Strict parsing:
    BOOST_REQUIRE(!uri_parse(
        "bitcoin:?label=Some ?????????", result, true));
}

BOOST_AUTO_TEST_CASE(uri_parse_message_test)
{
    // Message only:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse(
        "bitcoin:?message=Hi%20Alice", result));
    BOOST_REQUIRE(!result.address);
    BOOST_REQUIRE(!result.amount);
    BOOST_REQUIRE(!result.label);
    BOOST_REQUIRE(result.message && result.message.get() == "Hi Alice");
    BOOST_REQUIRE(!result.r);
}

BOOST_AUTO_TEST_CASE(uri_parse_payment_proto_test)
{
    // Payment protocol only:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse(
        "bitcoin:?r=http://www.example.com?purchase%3Dshoes", result));
    BOOST_REQUIRE(!result.address);
    BOOST_REQUIRE(!result.amount);
    BOOST_REQUIRE(!result.label);
    BOOST_REQUIRE(!result.message);
    BOOST_REQUIRE(result.r &&
        result.r.get() == "http://www.example.com?purchase=shoes");
}

BOOST_AUTO_TEST_CASE(uri_parse_unknown_test)
{
    // Unknown optional parameter:
    uri_parse_result result;
    BOOST_REQUIRE(uri_parse("bitcoin:?ignore=true", result));
    BOOST_REQUIRE(!result.address);
    BOOST_REQUIRE(!result.amount);
    BOOST_REQUIRE(!result.label);
    BOOST_REQUIRE(!result.message);
    BOOST_REQUIRE(!result.r);

    // Unknown required parameter:
    BOOST_REQUIRE(!uri_parse("bitcoin:?req-ignore=false", result));
}

BOOST_AUTO_TEST_CASE(uri_parse_custom_test)
{
    /**
    * Example class to demonstrate handling custom URI parameters.
    */
    struct custom_result
        : public uri_parse_result
    {
        optional_string myparam;

    protected:
        virtual bool got_param(std::string& key, std::string& value)
        {
            if ("myparam" == key)
                myparam.reset(value);
            return uri_parse_result::got_param(key, value);
        }
    };

    // Custom parameter type:
    custom_result custom;
    BOOST_REQUIRE(uri_parse("bitcoin:?myparam=here", custom));
    BOOST_REQUIRE(!custom.address);
    BOOST_REQUIRE(!custom.amount);
    BOOST_REQUIRE(!custom.label);
    BOOST_REQUIRE(!custom.message);
    BOOST_REQUIRE(!custom.r);
    BOOST_REQUIRE(custom.myparam && custom.myparam.get() == "here");
}

BOOST_AUTO_TEST_CASE(uri_write_test)
{
    uri_writer writer;
    writer.write_address(std::string("113Pfw4sFqN1T5kXUnKbqZHMJHN9oyjtgD"));
    writer.write_amount(120000);
    writer.write_amount(10000000000);
    writer.write_label("&=\n");
    writer.write_message("hello bitcoin");
    writer.write_r("http://example.com?purchase=shoes&user=bob");

    BOOST_REQUIRE_EQUAL(writer.string(),
        "bitcoin:113Pfw4sFqN1T5kXUnKbqZHMJHN9oyjtgD?"
        "amount=0.0012&amount=100&"
        "label=%26%3D%0A&"
        "message=hello%20bitcoin&"
        "r=http://example.com?purchase%3Dshoes%26user%3Dbob");
}

BOOST_AUTO_TEST_SUITE_END()
