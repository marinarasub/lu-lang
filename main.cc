#include "source.h"
#include "parse.h"
#include "expr.h"
#include "diag.h"

// TEST
//#include "adt/map.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <set>

#include "analyze.h"
#include "intermediate.h"
#include "interpreter.h"

//#include "adt/internal/avl.h"
#include "profile.h"
#include "rng.h"
#include "byte.h"
#include "internal/debug.h"

int main(int, char**)
{
    //lu::source src = lu::source::from_string("abc", "a: int32 = 3; $i32print (a), 123.99, \"abc\"\n(a, b) = (2, 3); $haha(me)");//"a = 3; 123.0; \"abc\"\n\nf: int -> (int = 0, s: int = 3, (int) = 7) = a -> (a, a, void); g: (X -> Y, A) -> B -> (C -> D) -> E\ne: () = ()\nxy: (x: int, y: int) #todo try defaulting values\na: int\n(b: float, c) <- (d, x) <- (1, 0); x, y = a, b = c = 4, d <- 6");//"x: int, y := 3, 2.0\n\n(x, y) <- (2, 3.0)\n(a,\nb\n); + a = 4; int(0, int(2.0, (), (2, 3), {})); ??? 234.0; 3331239(234); { {}\n{ (abc)(1); { def(); }\n }\n br @here\n { a; b; }; ret  \n br 3; }; br @there COND; ret @other\n\n ret @func expr \"a string that doesn't end { a = x; }");
    std::ifstream file("test.lu");
    lu::source src = lu::source::from_stream("test.lu", file);
    //std::cout << src2.size() << "\n" << lu::to_string(456).size() << lu::to_string(456).append("123") << "\n";

    try
    {
        lu::diag_logger log(lu::diag::DEBUG_LEVEL);
        lu::parse_expr_tree pet;
        if (!ok(lu::parse(&src, &pet, &log)))
        {
            log.flush();
            std::cout << "PARSE_FAIL" << "\n";
            return 1;
        }

        // TODO temp test
        // int x = 1234;
        // int y = 8642;
        // lu::intrinsics::i32add(&x, &y);
        // lu::intrinsics::i32print(nullptr, &x);

        lu::analyze_expr_tree aet;
        if (!ok(lu::analyze(&pet, &aet, &log)))
        {
            log.flush();
            std::cout << "ANALYZE_FAIL" << "\n";
            return 2;
        }

        lu::intermediate_program ip;
        if (!ok(lu::intermediate_transform(&aet, &ip, &log)))
        {
            log.flush();
            std::cout << "INTM_FAIL" << "\n";
            return 3;
        }

        log.flush();

        std::cout << ">>\n";

        lu::intermediate_interpreter_state iis;
        if (!ok(lu::interpret(&ip, &iis, 0, &log)))
        {
            log.flush();
            std::cout << "INTR_FAIL" << "\n";
            return 4;
        }

        log.flush();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    //std::cout << lu::to_string(pet);
    // lu::flat_map<int, char> map;
    // map.insert(0, 'a');
    // map.insert(2, 'b');
    // map.insert(1, '1');
    // map.insert(4, 'c');
    // map.insert(4, 'C');
    // map.insert(8, 'd');
    // map.insert(2, 'B');
    // map.insert(8, 'D');
    // assert(map.find(3) == map.end());
    // assert(map.find(-1) == map.end());
    // assert(map.find(7) == map.end());
    // assert(map.find(100) == map.end());
    // std::cout << map[0] << map[1] << map[2] << map[4] << map[8] << "\n";
    //using namespace lu::byte_size_literals;
    
    std::cout << "END" << "\n";
    return 0;
}
