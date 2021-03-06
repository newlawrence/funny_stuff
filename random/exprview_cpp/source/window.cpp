#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QWebChannel>

#include "window.hpp"


MainWindow::MainWindow(QWidget* parent) :
        QMainWindow{parent},
        _main_widget{new QWidget{}},
        _expression_box{new QLineEdit{""}},
        _infix_box{new QLineEdit{""}},
        _postfix_box{new QLineEdit{""}},
        _result_box{new QLineEdit{""}},
        _tree_view{new QWebEngineView{}},
        _error_handler{new ErrorHandler{this}},
        _tree_handler{new TreeHandler{this}}
{
    setWindowTitle("Expression Tree Viewer");
    setWindowIcon(QIcon{"qrc:/exprview.png"});
    resize(320, 480);

    auto create = [](auto label, auto box) noexcept {
        auto layout = new QHBoxLayout{};
        label->setBuddy(box);
        box->setAlignment(Qt::AlignRight);
        layout->addWidget(label);
        layout->addWidget(box);
        return layout;
    };

    auto form_layout = new QVBoxLayout{};
    auto expression_layout = create(new QLabel{"Expression:"}, _expression_box);
    auto infix_layout = create(new QLabel{"Infix notation:"}, _infix_box);
    auto postfix_layout = create(new QLabel{"Postfix notation:"}, _postfix_box);
    auto result_layout = create(new QLabel{"<b>Result:</b>"}, _result_box);

    auto box = QString{"qproperty-frame:false;background-color:transparent;"};
    auto highlight = QString{"font-weight:bold;color:red"};
    _infix_box->setStyleSheet("QLineEdit{" + box + "}");
    _infix_box->setReadOnly(true);
    _postfix_box->setStyleSheet("QLineEdit{" + box + "}");
    _postfix_box->setReadOnly(true);
    _result_box->setStyleSheet("QLineEdit{" + box + highlight + "}");
    _result_box->setReadOnly(true);

    auto line = new QFrame{};
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    form_layout->addLayout(expression_layout);
    form_layout->addWidget(line);

    form_layout->addLayout(infix_layout);
    form_layout->addLayout(postfix_layout);
    form_layout->addWidget(_tree_view);
    form_layout->addLayout(result_layout);

    _main_widget->setLayout(form_layout);
    setCentralWidget(_main_widget);

    auto channel = new QWebChannel(_tree_view->page());
    channel->registerObject("error_handler", _error_handler);
    channel->registerObject("tree_handler", _tree_handler);
    _tree_view->page()->setWebChannel(channel);
    _tree_view->page()->load(QUrl{"qrc:/index.html"});
    _tree_view->setContextMenuPolicy(Qt::NoContextMenu);

    auto bind = [&](auto method, auto box) noexcept {
        connect(_tree_handler, method, box, &QLineEdit::setText);
    };
    bind(&TreeHandler::infixChanged, _infix_box);
    bind(&TreeHandler::postfixChanged, _postfix_box);
    bind(&TreeHandler::resultChanged, _result_box);
    connect(
        _expression_box,
        &QLineEdit::textChanged,
        _tree_handler,
        &TreeHandler::renderTree
    );
    connect(
        _tree_handler,
        &TreeHandler::errorRaised,
        this,
        &MainWindow::handleError
    );
}

void MainWindow::handleError(const QString& text) {
    _infix_box->setText("");
    _postfix_box->setText("");
    _result_box->setText("");
    _error_handler->setText(text);
}
