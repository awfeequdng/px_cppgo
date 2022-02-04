#pragma once
#include <string>
#include <iostream>

#include "syntax/types/field_type.hh"

using namespace std;

namespace ast {

    typedef uint64_t uint64;
    typedef int64_t int64;

    struct Visitor;
    struct Node;

    using NodePtr = std::shared_ptr<Node>;
    // Node is the basic element of the AST.
    // Interfaces embed Node should have 'Node' name suffix.
    struct Node : public std::enable_shared_from_this<Node> {
        std::string text;
        // Accept accepts Visitor to visit itself.
        // The returned node should replace original node.
        // ok returns false to stop visiting.
        //
        // Implementation of this method should first call visitor.Enter,
        // assign the returned node to its method receiver, if skipChildren returns true,
        // children should be skipped. Otherwise, call its children in particular order that
        // later elements depends on former elements. Finally, return visitor.Leave.
        virtual bool Accept(Visitor *v, Node *node) = 0;
        // Text returns the original text of the element.
        virtual std::string Text() { return text; }
        // SetText sets original text to the Node.
        virtual void SetText(std::string txt) { this->text = txt; };

        virtual ~Node() = default;
    };


// Flags indicates whether an expression contains certain types of expression.

#define FlagConstant  0ul
#define FlagHasFunc   1ul << 0
#define FlagHasReference 1ul << 1
#define FlagHasAggregateFunc 1ul << 2
#define FlagHasVariable 1ul << 3
#define FlagHasDefault 1ul << 4

    // ExprNode is a node that can be evaluated.
    // Name of implementations should have 'Expr' suffix.
    struct ExprNode : Node {
        types::FieldType Type;
        uint64 flag;
        // SetType sets evaluation type to the expression.
        virtual void SetType(types::FieldTypePtr tp) {
            Type = *tp;
        }
        // GetType gets the evaluation type of the expression.
        virtual types::FieldTypePtr GetType() {
            return std::make_shared<types::FieldType>(Type);
        }
        // SetFlag sets flag to the expression.
        // Flag indicates whether the expression contains
        // parameter marker, reference, aggregate function...
        virtual void SetFlag(uint64 flg) {
            this->flag = flg;
        }
        // GetFlag returns the flag of the expression.
        virtual uint64 GetFlag() {
            return flag;
        }

        // Format formats the AST into a writer.
        virtual void Format(std::iostream &writer) = 0;
    };
    using ExprNodePtr = std::shared_ptr<ExprNode>;

    // OptBinary is used for syntax.
    struct OptBinary {
        bool IsBinary;
        std::string Charset;
    };

    // FuncNode represents function call expression node.
    struct FuncNode : ExprNode {
        virtual void functionExpression() {};
    };
    using FuncNodePtr = std::shared_ptr<FuncNode>;

    // StmtNode represents statement node.
    // Name of implementations should have 'Stmt' suffix.
    struct StmtNode : Node {
        virtual void statement() {};
    };
    using StmtNodePtr = std::shared_ptr<StmtNode>;

    // DDLNode represents DDL statement node.
    struct DDLNode : StmtNode {
        virtual void ddlStatement() {};
    };
    using DDLNodePtr = std::shared_ptr<DDLNode>;

    // DMLNode represents DML statement node.
    struct DMLNode : StmtNode {
        virtual void dmlStatement() {};
    };
    using DMLNodePtr = std::shared_ptr<DMLNode>;

    // ResultField represents a result field which can be a column from a table,
    // or an expression in select field. It is a generated property during
    // binding process. ResultField is the key element to evaluate a ColumnNameExpr.
    // After resolving process, every ColumnNameExpr will be resolved to a ResultField.
    // During execution, every row retrieved from table will set the row value to
    // ResultFields of that table, so ColumnNameExpr resolved to that ResultField can be
    // easily evaluated.
    struct ResultField {
//        model::ColumnInfoPtr Column;
//        model::CIStr ColumnAsName;
//        model::TableInfoPtr Table;
//        model::CIStr TableAsName;
//        model::CIStr DBName;

        // Expr represents the expression for the result field. If it is generated from a select field, it would
        // be the expression of that select field, otherwise the type would be ValueExpr and value
        // will be set for every retrieved row.
        ExprNodePtr Expr;
//        TableNamePtr TableName;
        // Referenced indicates the result field has been referenced or not.
        // If not, we don't need to get the values.
        bool Referenced;
    };
    using ResultFieldPtr = std::shared_ptr<ResultField>;

    // ResultSetNode interface has a ResultFields property, represents a Node that returns result set.
    // Implementations include SelectStmt, SubqueryExpr, TableSource, TableName and Join.
    struct ResultSetNode : Node {
    };
    using ResultSetNodePtr = std::shared_ptr<ResultSetNode>;


    // SensitiveStmtNode overloads StmtNode and provides a SecureText method.
    struct SensitiveStmtNode : StmtNode {
        // SecureText is different from Text that it hide password information.
        virtual std::string SecureText() = 0;
    };
    using SensitiveStmtNodePtr = std::shared_ptr<SensitiveStmtNode>;

    // Visitor visits a Node.
    struct Visitor {
        // Enter is called before children nodes are visited.
        // The returned node must be the same type as the input node n.
        // skipChildren returns true means children nodes should be skipped,
        // this is useful when work is done in Enter and there is no need to visit children.
        virtual bool Enter(NodePtr n, NodePtr node) = 0;
        // Leave is called after children nodes have been visited.
        // The returned node's type can be different from the input node if it is a ExprNode,
        // Non-expression node must be the same type as the input node n.
        // ok returns false to stop visiting.
        virtual bool Leave(NodePtr n, NodePtr node) = 0;
    };
    using VisitorPtr = std::shared_ptr<Visitor>;
}
