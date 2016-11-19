/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file parser.cpp
 * \date Nov 12, 2016
 */

#include "qsh/parser/parser.hpp"

#include <assert.h>
#include <fstream>
#include <sstream>

namespace qsh {

void set_error(void* p, void* l, char const* msg);

namespace {

#define YYDEBUG 1

#ifndef YYTOKENTYPE
# define YYTOKENTYPE
typedef parse_node_kind yytokentype;
#endif // YYTOKENTYPE

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

#include "tokenizer.gen"

static
void consume_comment(yyscan_t scanner, YYLTYPE* l)
{
    int c;
    while ((c = yyinput(scanner)) != 0) {
        if (c == '*') {
            while ((c = yyinput(scanner)) == '*') { }
            if (c == '/') {
                return;
            }
            if (c == 0) {
                break;
            }
        }
    }
    set_error(yyget_extra(scanner), l,
        "Line comment with regular comment "
        "not terminated (//* comment...)");
}

} // namespace

class parse_tree_builder_impl
{
    shared_ptr<arena> m_arena;
    shared_ptr<arena> m_text;
    std::vector<parse_node*> m_stack;
  public:
    parse_tree_builder_impl()
      : m_arena()
      , m_text()
      , m_stack()
    {
        m_arena = make_shared<arena>();
        m_text = make_shared<arena>();
    }

    void push(int line, int col, char const* lit, int len, parse_node_kind node_kind, int num_sub_nodes)
    {
        assert(num_sub_nodes <= (int)m_stack.size());
        auto n = reinterpret_cast<parse_node*>(m_arena->allocate(sizeof(parse_node) + num_sub_nodes*sizeof(parse_node*)));
        n->kind = (parse_node_kind)node_kind;
        n->num_nodes = num_sub_nodes;
        n->type = nullptr;
        n->line = line;
        n->col = col;
        n->text = m_text->create_string(lit, len);
        if (num_sub_nodes) {
            auto sn = n->sub();
            for (int i = num_sub_nodes-1; i >= 0; --i) {
                sn[i] = m_stack.back(); m_stack.pop_back();
            }
        }
        m_stack.push_back(n);
    }

    void push_var(int line, int col, char const* lit, int len, bool is_arr, int num_sub)
    {
        assert(1 <= (int)m_stack.size());
        auto n = reinterpret_cast<parse_node*>(m_arena->allocate(sizeof(parse_node) + (num_sub+1)*sizeof(parse_node*)));
        n->kind = VAR_DEF;
        n->num_nodes = 1 + num_sub;
        n->type = nullptr;
        n->line = line;
        n->col = col;
        n->text = m_text->create_string(lit, len);
        auto sn = n->sub();
        sn[0] = reinterpret_cast<parse_node*>(is_arr ? 1 : 0);
        for (int i = num_sub; i >= 1; --i) {
            sn[i] = m_stack.back(); m_stack.pop_back();
        }
        m_stack.push_back(n);
    }

    void push_fn(int line, int col, char const* lit, int len, parse_node_kind node_kind, int num_sub_nodes, int num_args)
    {
        assert(num_sub_nodes <= (int)m_stack.size());
        auto n = reinterpret_cast<parse_node*>(m_arena->allocate(sizeof(parse_node) + sizeof(size_t) + num_sub_nodes*sizeof(parse_node*)));
        n->kind = (parse_node_kind)node_kind;
        n->num_nodes = num_sub_nodes+1;
        n->type = nullptr;
        n->line = line;
        n->col = col;
        n->text = m_text->create_string(lit, len);
        if (num_sub_nodes) {
            auto sn = n->sub();
            sn[0] = reinterpret_cast<parse_node*>(num_args);
            for (int i = num_sub_nodes; i >= 1; --i) {
                sn[i] = m_stack.back(); m_stack.pop_back();
            }
        }
        m_stack.push_back(n);
    }

    parse_tree release()
    {
        parse_tree p;
        push(0, 0, "", 0, TREE_ROOT, m_stack.size());
        p.m_root = m_stack[0];
        p.m_arena = m_arena;
        p.m_text = m_text;
        m_arena = make_shared<arena>();
        m_text = make_shared<arena>();
        m_stack.resize(0);
        return p;
    }
};

struct parse_tree_builder
{
    parse_tree_builder_impl m_impl;

    void push_node(int line, int col, char const* lit, int len, parse_node_kind kind, int num_sub_nodes)
    {
        m_impl.push(line, col, lit, len, kind, num_sub_nodes);
    }

    void push_fn_node(int line, int col, char const* lit, int len, parse_node_kind node_kind, int num_sub_nodes, int num_args)
    {
        m_impl.push_fn(line, col, lit, len, node_kind, num_sub_nodes, num_args);
    }

    void push_var_node(int line, int col, char const* lit, int len, bool is_arr, int num_sub)
    {
        m_impl.push_var(line, col, lit, len, is_arr, num_sub);
    }

    parse_tree release()
    {
        return m_impl.release();
    }
};

class string_view : public const_range<char>
{
  public:
    string_view(char const* str, ptrdiff_t len) : const_range<char>(str, str+len)
    { }
};

class parser_impl : noncopyable
{
    yyscan_t m_scan;
    YY_BUFFER_STATE m_buff;
    parse_tree_builder m_builder;

    YYLTYPE m_loc;
    char const* m_text;
    ptrdiff_t m_len;
    int m_token;
    std::string m_err;
    mutable std::string m_errfinal;

  public:
    parser_impl() : m_scan(), m_builder(), m_text(), m_len(-1), m_token()
    {
        auto fail = yylex_init(&m_scan);
        assert(!fail);
        yyset_debug(YYDEBUG, m_scan);
        yyset_extra(&m_builder, m_scan);
    }

    ~parser_impl()
    {
        yylex_destroy(m_scan);
    }

    parse_tree release_tree()
    {
        return m_builder.release();
    }

    void set_err(YYLTYPE* l, char const* msg)
    {
        m_loc = *l;
        m_err = msg;
    }

  private:
    void push_token(parse_node_kind kind, int num_sub = 0)
    {
        m_builder.push_node(m_loc.first_line, m_loc.first_column, m_text, m_len, kind, num_sub);
    }
    void push_func(int num_sub, int num_args)
    {
        m_builder.push_fn_node(m_loc.first_line, m_loc.first_column, m_text, m_len, FUNC_DEF, num_sub, num_args);
    }
    void push_var(bool is_arr, int num_sub)
    {
        m_builder.push_var_node(m_loc.first_line, m_loc.first_column, m_text, m_len, is_arr, num_sub);
    }

    template <int N>
    void push_token(char const (&lit)[N], parse_node_kind kind, int num_sub = 0)
    {
        m_builder.push_node(m_loc.first_line, m_loc.first_column, lit, N-1, kind, num_sub);
    }

    bool has_token() const
    {
        return m_len != -1;
    }

    void next_token()
    {
        if ((m_token = yylex(&m_token, &m_loc, m_scan)) == 0) {
            m_len = -1;
        } else {
            m_len = yyget_leng(m_scan);
            m_text = yyget_text(m_scan);
        }
    }

    YYLTYPE location() const
    {
        return m_loc;
    }

    string_view text() const
    {
        return string_view(m_text, m_len);
    }

    int token() const
    {
        return m_token;
    }

//////////////////////////////////////////////////

#define check_eos(msg)      \
    if (!has_token()) {     \
        m_err = msg;        \
        m_err.append(" Unexpected end of file."); \
        return false;       \
    }
#define check_char(c) \
    check_eos("Expected character " #c ".");  \
    {                                           \
        auto t = text();                        \
        if (t.size() != 1 || t[0] != c) {       \
            m_err = "Expected character " #c "."; \
            m_err.append(" Found `");           \
            m_err.append(t.begin(), t.size());  \
            m_err.append("' instead.");         \
            return false;                       \
        }                                       \
    }

    bool read_identifier()
    {
        check_eos("Expected identifier. Unexpected end of file.");
        if (token() == IDENT) {
            push_token(IDENT);
            next_token();
            return true;
        } else {
            m_err = "Expected a valid identifier name. "
                    "Any sequence of a-z,A-Z,0-9,_ that does not start"
                    " with a number is a valid identifier name.";
            return false;
        }
    }

    bool read_constant()
    {
        bool rv = false;
        switch (token()) {
        case LIT_CHAR:
            rv = true;
            push_token(LIT_CHAR);
            next_token();
            break;
        case LIT_STRING:
            rv = true;
            push_token(LIT_STRING);
            next_token();
            break;
        case LIT_INT_HEX:
            rv = true;
            push_token(LIT_INT_HEX);
            next_token();
            break;
        case LIT_INT_DEC:
            rv = true;
            push_token(LIT_INT_DEC);
            next_token();
            break;
        case LIT_FLOAT:
            rv = true;
            push_token(LIT_FLOAT);
            next_token();
            break;
        default:
            break;
        }
        return rv;
    }

    bool read_unary()
    {
        bool rv = false;
        switch (token()) {
            case LIT_CHAR:
            case LIT_STRING:
            case LIT_INT_HEX:
            case LIT_INT_DEC:
            case LIT_FLOAT:
            case IDENT:
                rv = true;
                push_token((parse_node_kind)token());
                next_token();
                break;
            case OP_INCR:
            case OP_DECR:
            case TOK_PLUS:
            case TOK_MINUS:
            case TOK_FLIP:
            case TOK_NOT:
                rv = true;
                push_token((parse_node_kind)token());
                next_token();
                if (!read_expr()) {
                    rv = false;
                }
                break;
            default:
                break;
        }

        return rv;
    }

    bool read_array_init()
    {
        check_char('{');
        next_token();
        int num_el = 0;
        while (has_token()) {
            if (!read_expr()) {
                return false;
            }
            ++num_el;
            if (token() != TOK_COMMA) {
                break;
            }
        }
        check_char('}');
        push_token(ARRAY_INIT, num_el);
        next_token();
        return true;
    }

    bool read_formal()
    {
        assert(token() == KWD_VAR);
        next_token();
        check_eos("Expected a variable declaration as function parameter.");
        return read_identifier();
    }

    bool read_return_stmt()
    {
        assert(token() == KWD_RETURN);
        next_token();
        bool rv = false;
        if (has_token()) {
            if (token() == TOK_SEMI) {
                push_token(KWD_RETURN);
                next_token();
                rv = true;
            } else if (read_expr()) {
                if (token() == TOK_SEMI) {
                    rv = true;
                    push_token(KWD_RETURN, 1);
                    next_token();
                } else {
                    m_err = "Expected return statement to terminate with ';'";
                }
            }
        }
        if (!rv && m_err.empty()) {
            m_err = "Expected a valid return statement:  return; or return <expr>;";
        }
        return rv;
    }

    bool read_expr()
    {
        check_eos("Expected an expression.");
        if (token() == IDENT) {
            push_token(IDENT);
            next_token();
            if (token() == TOK_SQ_L) {
                next_token();
                if (!read_expr()) {
                    return false;
                }
                push_token(EXPR_INDEX, 2);
                check_char(']');
                next_token();
                return true;
            } else {
                return true;
            }
        }
        if (read_unary()) {
            return true;
        }
        m_err = "Expected an expression.";
        return false;
    }

    bool read_statement()
    {
        switch (token()) {
        case KWD_VAR:
            return read_var_def();
        case KWD_RETURN:
            return read_return_stmt();
        default:
            return read_expr();
        }
    }

    // id '=' expr
    bool read_assign()
    {
        if (!read_identifier()) {
            return false;
        }
        check_char('=');
        next_token();
        if (!read_expr()) {
            return false;
        }
        push_token("=", TOK_ASSIGN, 2);
        return true;
    }

    bool read_func_def()
    {
        assert(token() == KWD_DEF);
        next_token();
        if (!read_identifier()) {
            return false;
        }
        check_char('(');
        next_token();
        int nargs = 0;
        while (token() == KWD_VAR) {
            if (!read_formal()) {
                return false;
            }
            ++nargs;
            if (token() != TOK_COMMA) {
                break;
            } else {
                next_token();
            }
        }
        check_char(')'); next_token();
        check_char('{'); next_token();
        int nstmts = 0;
        while (has_token() && token() != TOK_CURLY_R) {
            if (!read_statement()) {
                return false;
            }
            ++nstmts;
        }
        push_func(nstmts+nargs+1/*funcname*/, nargs);
        check_char('}'); next_token();
        return true;
    }

    bool read_var_def()
    {
        assert(token() == KWD_VAR);
        next_token();
        if (!read_identifier()) {
            return false;
        }

        bool was_array = false;
        if (token() == TOK_SQ_L) {
            next_token();
            check_char(']');
            next_token();
            was_array = true;
        }

        check_char('=');
        next_token();

        if (was_array) {
            if (!read_array_init()) {
                return false;
            }
        } else {
            if (!read_expr()) {
                return false;
            }
        }

        push_var(was_array, 2);
        check_char(';');
        next_token();
        return true;
    }

    bool read_external_decl()
    {
        switch (token()) {
        case KWD_DEF:
            return read_func_def();
        case KWD_VAR:
            return read_var_def();
        default:
            m_err = "Expected variable or function definition not found.";
            return false;
        }
    }
  public:
    char const* error_msg() const
    {
        if (m_errfinal.empty() && !m_err.empty()) {
            m_errfinal.append("line ");
            m_errfinal.append(std::to_string(m_loc.first_line));
            m_errfinal.append(" col ");
            m_errfinal.append(std::to_string(m_loc.first_column));
            m_errfinal.append(": ");
            m_errfinal.append(m_err);
        }
        return m_errfinal.c_str();
    }

    bool parse_string(char const* str, ptrdiff_t len)
    {
        if (len == -1) {
            len = strlen(str);
        }
        m_loc.first_line = 1;
        m_loc.first_column = 1;
        m_loc.last_line = 2;
        m_loc.last_column = 1;
        m_buff = yy_scan_bytes(str, len, m_scan);
        yyset_lineno(1, m_scan);
        yyset_column(1, m_scan);
        m_err.clear();
        m_errfinal.clear();
        next_token();
        bool succ = true;
        while (has_token() && succ) {
            succ = read_external_decl();
        }
        yy_delete_buffer(m_buff, m_scan);
        return succ;
    }
};

void set_error(void* p, void* l, char const* msg)
{
    ((parser_impl*)p)->set_err((YYLTYPE*)l, msg);
}

parser::parser() : m_impl()
{
    static_assert(sizeof(parser_impl) <= IMPL_SIZE, "Parser impl is sliced!");
    m_impl.reset();
}

parser::~parser()
{
}

parse_tree parser::parse_file(char const* filename)
{
    std::string contents;
    {// read file into string
        std::ifstream file(filename, std::ios::binary);
        std::ostringstream strm;
        strm << file.rdbuf();
        contents = strm.str(); 
    }
    return parse_string(contents.c_str(), contents.size());
}

parse_tree parser::parse_string(char const* str, ptrdiff_t len)
{
    if (m_impl->parse_string(str, len)) {
        return m_impl->release_tree();
    }
    return parse_tree();
}

char const* parser::err_msg() const
{
    return m_impl->error_msg();
}

} // namespace qsh
