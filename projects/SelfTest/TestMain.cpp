/*
 *  Created by Phil on 22/10/2010.
 *  Copyright 2010 Two Blue Cubes Ltd
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wpadded"
#endif

#include "catch_self_test.hpp"

TEST_CASE( "selftest/main", "Runs all Catch self tests and checks their results" ) {
    using namespace Catch;

    ///////////////////////////////////////////////////////////////////////////
    SECTION(    "selftest/expected result",
                "Tests do what they claim" ) {
                
        SECTION(    "selftest/expected result/failing tests", 
                    "Tests in the 'failing' branch fail" ) {
            MetaTestRunner::runMatching( "./failing/*",  MetaTestRunner::Expected::ToFail );
        }
        
        SECTION(    "selftest/expected result/succeeding tests", 
                    "Tests in the 'succeeding' branch succeed" ) {
            MetaTestRunner::runMatching( "./succeeding/*",  MetaTestRunner::Expected::ToSucceed );
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    SECTION(    "selftest/test counts", 
                "Number of test cases that run is fixed" ) {
        EmbeddedRunner runner;
        
        SECTION(    "selftest/test counts/succeeding tests", 
                    "Number of 'succeeding' tests is fixed" ) {
            Totals totals = runner.runMatching( "./succeeding/*" );
            CHECK( totals.assertions.passed == 294 );
            CHECK( totals.assertions.failed == 0 );
        }

        SECTION(    "selftest/test counts/failing tests", 
                    "Number of 'failing' tests is fixed" ) {
            Totals totals = runner.runMatching( "./failing/*" );        
            CHECK( totals.assertions.passed == 1 );
            CHECK( totals.assertions.failed == 73 );
        }
    }
}

TEST_CASE( "meta/Misc/Sections", "looped tests" ) {
    Catch::EmbeddedRunner runner;
    
    Catch::Totals totals = runner.runMatching( "./mixed/Misc/Sections/nested2" );
    CHECK( totals.assertions.passed == 2 );
    CHECK( totals.assertions.failed == 1 );
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wweak-vtables"
#endif

#include "../../include/internal/catch_commandline.hpp"
#include "../../include/internal/catch_test_spec.h"
#include "../../include/reporters/catch_reporter_basic.hpp"
#include "../../include/reporters/catch_reporter_xml.hpp"
#include "../../include/reporters/catch_reporter_junit.hpp"

template<size_t size>
void parseIntoConfig( const char * (&argv)[size], Catch::ConfigData& config ) {
    static Catch::AllOptions options;
    options.parseIntoConfig( Catch::CommandParser( size, argv ), config );
}

template<size_t size>
std::string parseIntoConfigAndReturnError( const char * (&argv)[size], Catch::ConfigData& config ) {
    try {
        parseIntoConfig( argv, config );
        FAIL( "expected exception" );
    }
    catch( std::exception& ex ) {
        return ex.what();
    }
    return "";
}

inline Catch::TestCase fakeTestCase( const char* name ){ return Catch::makeTestCase( NULL, "", name, "", CATCH_INTERNAL_LINEINFO ); }

TEST_CASE( "selftest/parser/2", "ConfigData" ) {

    Catch::ConfigData config;

    SECTION( "default", "" ) {
        const char* argv[] = { "test" };
        CHECK_NOTHROW( parseIntoConfig( argv, config ) );
        
        CHECK( config.shouldDebugBreak == false );
        CHECK( config.cutoff == -1 );
        CHECK( config.allowThrows == true );
        CHECK( config.reporter.empty() );
    }
    
    SECTION( "test lists", "" ) {
        SECTION( "-t/1", "Specify one test case using -t" ) {
            const char* argv[] = { "test", "-t", "test1" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.filters.size() == 1 );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "notIncluded" ) ) == false );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "test1" ) ) );
        }
        SECTION( "-t/exclude:1", "Specify one test case exclusion using -t exclude:" ) {
            const char* argv[] = { "test", "-t", "exclude:test1" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.filters.size() == 1 );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "test1" ) ) == false );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "alwaysIncluded" ) ) );
        }

        SECTION( "--test/1", "Specify one test case using --test" ) {
            const char* argv[] = { "test", "--test", "test1" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.filters.size() == 1 );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "notIncluded" ) ) == false );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "test1" ) ) );
        }

        SECTION( "--test/exclude:1", "Specify one test case exclusion using --test exclude:" ) {
            const char* argv[] = { "test", "--test", "exclude:test1" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.filters.size() == 1 );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "test1" ) ) == false );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "alwaysIncluded" ) ) );
        }

        SECTION( "--test/exclude:2", "Specify one test case exclusion using --test ~" ) {
            const char* argv[] = { "test", "--test", "~test1" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.filters.size() == 1 );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "test1" ) ) == false );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "alwaysIncluded" ) ) );
        }
        
        SECTION( "-t/2", "Specify two test cases using -t" ) {
            const char* argv[] = { "test", "-t", "test1", "test2" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.filters.size() == 1 );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "notIncluded" ) ) == false );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "test1" ) ) );
            REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "test2" ) ) );
        }

        SECTION( "-t/0", "When no test names are supplied it is an error" ) {
            const char* argv[] = { "test", "-t" };
            REQUIRE_THAT( parseIntoConfigAndReturnError( argv, config ), Contains( "at least 1" ) );
        }
    }
    
    SECTION( "reporter", "" ) {
        SECTION( "-r/basic", "" ) {
            const char* argv[] = { "test", "-r", "basic" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.reporter == "basic" );
        }
        SECTION( "-r/xml", "" ) {
            const char* argv[] = { "test", "-r", "xml" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.reporter == "xml" );
        }
        SECTION( "--reporter/junit", "" ) {
            const char* argv[] = { "test", "--reporter", "junit" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.reporter == "junit" );
        }
        SECTION( "-r/error", "reporter config only accepts one argument" ) {
            const char* argv[] = { "test", "-r", "one", "two" };
            REQUIRE_THAT( parseIntoConfigAndReturnError( argv, config ), Contains( "1 argument" ) );
        }
    }
    
    SECTION( "debugger", "" ) {
        SECTION( "-b", "" ) {
            const char* argv[] = { "test", "-b" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.shouldDebugBreak == true );
        }
        SECTION( "--break", "" ) {
            const char* argv[] = { "test", "--break" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.shouldDebugBreak );
        }
        SECTION( "-b", "break option has no arguments" ) {
            const char* argv[] = { "test", "-b", "unexpected" };
            REQUIRE_THAT( parseIntoConfigAndReturnError( argv, config ), Contains( "0 arguments" ) );
        }
    }
        
    SECTION( "abort", "" ) {
        SECTION( "-a", "" ) {
            const char* argv[] = { "test", "-a" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.cutoff == 1 );
        }
        SECTION( "-a/2", "" ) {
            const char* argv[] = { "test", "-a", "2" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.cutoff == 2 );
        }
        SECTION( "-a/error/0", "" ) {
            const char* argv[] = { "test", "-a", "0" };
            REQUIRE_THAT( parseIntoConfigAndReturnError( argv, config ), Contains( "greater than zero" ) );
        }
        SECTION( "-a/error/non numeric", "" ) {
            const char* argv[] = { "test", "-a", "oops" };
            REQUIRE_THAT( parseIntoConfigAndReturnError( argv, config ), Contains( "greater than zero" ) );
        }
        SECTION( "-a/error/two args", "cutoff only takes one argument" ) {
            const char* argv[] = { "test", "-a", "1", "2" };
            REQUIRE_THAT( parseIntoConfigAndReturnError( argv, config ), Contains( "0 and 1 argument" ) );
        }
    }
    
    SECTION( "nothrow", "" ) {
        SECTION( "-nt", "" ) {
            const char* argv[] = { "test", "-nt" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.allowThrows == false );
        }
        SECTION( "--nothrow", "" ) {
            const char* argv[] = { "test", "--nothrow" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.allowThrows == false );
        }
    }

    SECTION( "streams", "" ) {
        SECTION( "-o filename", "" ) {
            const char* argv[] = { "test", "-o", "filename.ext" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.outputFilename == "filename.ext" );
            REQUIRE( config.stream.empty() );
        }
        SECTION( "-o %stdout", "" ) {
            const char* argv[] = { "test", "-o", "%stdout" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.stream == "stdout" );
            REQUIRE( config.outputFilename.empty() );
        }
        SECTION( "--out", "" ) {
            const char* argv[] = { "test", "--out", "filename.ext" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.outputFilename == "filename.ext" );
        }
    }

    SECTION( "combinations", "" ) {
        SECTION( "-a -b", "" ) {
            const char* argv[] = { "test", "-a", "-b", "-nt" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            CHECK( config.cutoff == 1 );
            CHECK( config.shouldDebugBreak );
            CHECK( config.allowThrows == false );
        }
    }        
}

TEST_CASE( "selftest/test filter", "Individual filters" ) {

    Catch::TestCaseFilter matchAny( "*" );
    Catch::TestCaseFilter matchNone( "*", Catch::IfFilterMatches::ExcludeTests );
    CHECK( matchAny.shouldInclude( fakeTestCase( "any" ) ));
    CHECK( matchNone.shouldInclude( fakeTestCase( "any" ) ) == false );

    Catch::TestCaseFilter matchHidden( "./*" );
    Catch::TestCaseFilter matchNonHidden( "./*", Catch::IfFilterMatches::ExcludeTests );

    CHECK( matchHidden.shouldInclude( fakeTestCase( "any" ) ) == false );
    CHECK( matchNonHidden.shouldInclude( fakeTestCase( "any" ) ) );

    CHECK( matchHidden.shouldInclude( fakeTestCase( "./any" ) ) );
    CHECK( matchNonHidden.shouldInclude( fakeTestCase( "./any" ) ) == false );
}

TEST_CASE( "selftest/test filters", "Sets of filters" ) {

    Catch::TestCaseFilter matchHidden( "./*" );
    Catch::TestCaseFilter dontMatchA( "./a*", Catch::IfFilterMatches::ExcludeTests );
    Catch::TestCaseFilters filters( "" );
    filters.addFilter( matchHidden );
    filters.addFilter( dontMatchA );

    CHECK( matchHidden.shouldInclude( fakeTestCase( "./something" ) ) );

    CHECK( filters.shouldInclude( fakeTestCase( "any" ) ) == false );
    CHECK( filters.shouldInclude( fakeTestCase( "./something" ) ) );
    CHECK( filters.shouldInclude( fakeTestCase( "./anything" ) ) == false );
}

TEST_CASE( "selftest/filter/prefix wildcard", "Individual filters with wildcards at the start" ) {
    Catch::TestCaseFilter matchBadgers( "*badger" );

    CHECK( matchBadgers.shouldInclude( fakeTestCase( "big badger" ) ));
    CHECK( matchBadgers.shouldInclude( fakeTestCase( "little badgers" ) ) == false );
}
TEST_CASE( "selftest/filter/wildcard at both ends", "Individual filters with wildcards at both ends" ) {
    Catch::TestCaseFilter matchBadgers( "*badger*" );

    CHECK( matchBadgers.shouldInclude( fakeTestCase( "big badger" ) ));
    CHECK( matchBadgers.shouldInclude( fakeTestCase( "little badgers" ) ) );
    CHECK( matchBadgers.shouldInclude( fakeTestCase( "badgers are big" ) ) );
    CHECK( matchBadgers.shouldInclude( fakeTestCase( "hedgehogs" ) ) == false );
}


template<size_t size>
int getArgc( const char * (&)[size] ) {
    return size;
}

TEST_CASE( "selftest/option parsers", "" )
{
    Catch::ConfigData config;
    
    Catch::SharedImpl<Catch::Options::TestCaseOptionParser> tcOpt;
    Catch::OptionParser& opt = tcOpt;

    const char* argv[] = { "test", "-t", "test1" };

    Catch::CommandParser parser( getArgc( argv ), argv );

    CHECK_NOTHROW( opt.parseIntoConfig( parser, config ) );

    REQUIRE( config.filters.size() == 1 );
    REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "notIncluded" ) ) == false );
    REQUIRE( config.filters[0].shouldInclude( fakeTestCase( "test1" ) ) );
}

TEST_CASE( "selftest/tags", "" ) {

    std::string p1 = "[one]";
    std::string p2 = "[one],[two]";
    std::string p3 = "[one][two]";
    std::string p4 = "[one][two],[three]";
    std::string p5 = "[one][two]~[hide],[three]";
    
    SECTION( "one tag", "" ) {
        Catch::TestCase oneTag = makeTestCase( NULL, "", "test", "[one]", CATCH_INTERNAL_LINEINFO );

        CHECK( oneTag.getTestCaseInfo().description == "" );
        CHECK( oneTag.hasTag( "one" ) );
        CHECK( oneTag.getTags().size() == 1 );

        CHECK( oneTag.matchesTags( p1 ) == true );
        CHECK( oneTag.matchesTags( p2 ) == true );
        CHECK( oneTag.matchesTags( p3 ) == false );
        CHECK( oneTag.matchesTags( p4 ) == false );
        CHECK( oneTag.matchesTags( p5 ) == false );
    }

    SECTION( "two tags", "" ) {
        Catch::TestCase twoTags= makeTestCase( NULL, "", "test", "[one][two]", CATCH_INTERNAL_LINEINFO );

        CHECK( twoTags.getTestCaseInfo().description == "" );
        CHECK( twoTags.hasTag( "one" ) );
        CHECK( twoTags.hasTag( "two" ) );
        CHECK( twoTags.hasTag( "three" ) == false );
        CHECK( twoTags.getTags().size() == 2 );

        CHECK( twoTags.matchesTags( p1 ) == true );
        CHECK( twoTags.matchesTags( p2 ) == true );
        CHECK( twoTags.matchesTags( p3 ) == true );
        CHECK( twoTags.matchesTags( p4 ) == true );
        CHECK( twoTags.matchesTags( p5 ) == true );
    }

    SECTION( "one tag with characters either side", "" ) {

        Catch::TestCase oneTagWithExtras = makeTestCase( NULL, "", "test", "12[one]34", CATCH_INTERNAL_LINEINFO );
        CHECK( oneTagWithExtras.getTestCaseInfo().description == "1234" );
        CHECK( oneTagWithExtras.hasTag( "one" ) );
        CHECK( oneTagWithExtras.hasTag( "two" ) == false );
        CHECK( oneTagWithExtras.getTags().size() == 1 );
    }
    
    SECTION( "start of a tag, but not closed", "" ) {

        Catch::TestCase oneTagOpen = makeTestCase( NULL, "", "test", "[one", CATCH_INTERNAL_LINEINFO );

        CHECK( oneTagOpen.getTestCaseInfo().description == "[one" );
        CHECK( oneTagOpen.hasTag( "one" ) == false );
        CHECK( oneTagOpen.getTags().size() == 0 );
    }

    SECTION( "hidden", "" ) {
        Catch::TestCase oneTag = makeTestCase( NULL, "", "test", "[hide]", CATCH_INTERNAL_LINEINFO );

        CHECK( oneTag.getTestCaseInfo().description == "" );
        CHECK( oneTag.hasTag( "hide" ) );
        CHECK( oneTag.isHidden() );

        CHECK( oneTag.matchesTags( "~[hide]" ) == false );

    }
    
}
