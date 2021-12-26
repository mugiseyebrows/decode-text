#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextCodec>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QFile>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QList<QByteArray> codecs = QTextCodec::availableCodecs();
    QStringList codecs_;
    for(const auto& name: codecs) {
        codecs_.append(QString::fromLatin1(name));
    }
    QStringListModel* model = new QStringListModel(codecs_);

    ui->codec1->setModel(model);
    ui->codec2->setModel(model);

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool isText(const QString& text) {
    QString alphabet = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
                       "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "abcdefghijklmnopqrstuvwxyz";
    for(int i=0;i<text.size();i++) {
        if (alphabet.contains(text.mid(i,1))) {
            return true;
        }
    }
    return false;
}

void dumpCodecs(const QString& path) {
    QList<QByteArray> codecs = QTextCodec::availableCodecs();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "cannot open file" << path;
    }
    for(const auto& name: codecs) {
        file.write(name);
        file.write("\n");
    }
    file.close();
}

void MainWindow::on_decode_clicked()
{
    auto* codec1 = QTextCodec::codecForName(ui->codec1->currentText().toLatin1());
    auto* codec2 = QTextCodec::codecForName(ui->codec2->currentText().toLatin1());
    auto* decoder = codec2->makeDecoder(QTextCodec::ConvertInvalidToNull);
    QByteArray encoded = codec1->fromUnicode(ui->encoded->toPlainText());
    QString decoded = decoder->toUnicode(encoded);
    ui->decoded->setPlainText(decoded);
}

void MainWindow::on_guess_clicked()
{
    QString sample = ui->sample->text();

    if (sample.isEmpty()) {
        return;
    }

    QList<QByteArray> codecs = QTextCodec::availableCodecs();

    if (ui->popular->isChecked()) {
        codecs = {"UTF-8", "ISO-8859-1", "UTF-16", "CP1251", "CP866", "KOI8-R"};
    } else {
        codecs = QTextCodec::availableCodecs();
    }

    //dumpCodecs("D:\\w\\codecs.txt");

    QList<QStringList> res;

    for(const auto& name1: codecs) {
        for (const auto& name2: codecs) {
            auto* codec1 = QTextCodec::codecForName(name1);
            auto* codec2 = QTextCodec::codecForName(name2);
            auto* decoder = codec2->makeDecoder(QTextCodec::ConvertInvalidToNull);
            QString text = decoder->toUnicode(codec1->fromUnicode(sample));
            if (isText(text)) {
                QStringList item;
                item << QString::fromLatin1(name1) << QString::fromLatin1(name2) << text;
                res.append(item);
            }
        }
    }

    QStandardItemModel* model = new QStandardItemModel(res.size(), 3, ui->decoded);
    for(int row=0;row<res.size();row++) {
        for (int column=0;column<3;column++) {
            QStandardItem* item = new QStandardItem(res[row][column]);
            item->setEditable(false);
            model->setItem(row,column, item);
        }
    }
    model->setHorizontalHeaderLabels({"codec1","codec2","sample"});

    ui->guessed->setModel(model);
}

void MainWindow::on_guessed_doubleClicked(const QModelIndex &index)
{
    QAbstractItemModel* model = ui->guessed->model();

    QString codec1 = model->data(model->index(index.row(), 0)).toString();
    QString codec2 = model->data(model->index(index.row(), 1)).toString();
    ui->codec1->setCurrentText(codec1);
    ui->codec2->setCurrentText(codec2);

    auto text = ui->sample->text();
    text.replace(QChar('\0'), QChar(' '));

    ui->iconv->setText(QString("echo \"%1\" | iconv -t %2 | iconv -f %3").arg(text).arg(codec1).arg(codec2));

}
