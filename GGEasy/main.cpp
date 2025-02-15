// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/

#include "gc_fileplugin.h"
#include "gc_plugin.h"
#include "splashscreen.h"

#include "mainwindow.h"
#include "settingsdialog.h"
#include "shapepluginin.h"
#include "version.h"
#include <QCommandLineParser>
#include <QDir>
#include <QPluginLoader>
#include <QStandardPaths>
#include <QSystemSemaphore>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

#include <algorithm>
#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QByteArray localMsg = msg.toUtf8();
    const char* file = context.file ? context.file : "";
    //    const char* function = context.function ? context.function : "";

    const char* file_ {file};
    while (*file > 0) {
        if (*file == '\\')
            file_ = file + 1;
        ++file;
    }
    file = file_;

    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "Debug: %s\n\t%s : %u\n", localMsg.constData(), file, context.line /*, function*/);
        break;
    case QtInfoMsg:
        fprintf(stdout, "Info: %s\n\t%s : %u\n", localMsg.constData(), file, context.line /*, function*/);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning:%s\n\t%s : %u\n", localMsg.constData(), file, context.line /*, function*/);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical:%s\n\t%s : %u\n", localMsg.constData(), file, context.line /*, function*/);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal:%s\n\t%s : %u\n", localMsg.constData(), file, context.line /*, function*/);
        break;
    }
}

int main(int argc, char** argv) {
    //    qInstallMessageHandler(myMessageOutput);

#ifdef LEAK_DETECTOR
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    // qInstallMessageHandler(myMessageOutput);
    QApplication::setAttribute(Qt::AA_Use96Dpi);
    qputenv("QT_ENABLE_HIGHDPI_SCALING", QByteArray("0"));

    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);

//    DoubleSpinBox dsbx;
//    dsbx.resize(500, 50);
//    dsbx.setRange(-9999, +9999);
//    dsbx.show();
//    dsbx.setSuffix(" mm");
//    return app.exec();

    //#ifdef Q_OS_WIN
    //    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    //    if (settings.value("AppsUseLightTheme") == 0) {
    //        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    //    }
    //#endif

#ifdef linux
    // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
    // поэтому необходимо избавиться от данного мусора
    QSharedMemory nixFixSharedMemory("AppSettings");
    if (nixFixSharedMemory.attach())
        nixFixSharedMemory.detach();
#endif
    QApplication::setApplicationName("GGEasy");
    QApplication::setOrganizationName(VER_COMPANYNAME_STR);
    QApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    [[maybe_unused]] App appSingleton;
    App::settingsPath() = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).front();

    if (QDir dir(App::settingsPath()); !dir.exists())
        dir.mkpath(App::settingsPath());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    // QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, "");

    //  FIXME ??? QGLFormat glf = QGLFormat::defaultFormat();
    //    glf.setSampleBuffers(true);
    //    glf.setSamples(16);
    //    QGLFormat::setDefaultFormat(glf);

    if constexpr (0) {
        QSystemSemaphore semaphore("GGEasySemaphore", 1); // создаём семафор
        semaphore.acquire();                              // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью
#ifdef linux
        // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
        // поэтому необходимо избавиться от данного мусора
        QSharedMemory nix_fix_shared_memory("GGEasyMemory");
        if (nix_fix_shared_memory.attach())
            nix_fix_shared_memory.detach();
#endif
        MainWindow* mainWin = nullptr;
        QSharedMemory sharedMemory("GGEasyMemory"); // Создаём экземпляр разделяемой памяти
        auto instance = [&sharedMemory]() -> MainWindow*& { return *static_cast<MainWindow**>(sharedMemory.data()); };
        bool is_running = false;     // переменную для проверки ууже запущенного приложения
        if (sharedMemory.attach()) { // пытаемся присоединить экземпляр разделяемой памяти к уже существующему сегменту
            is_running = true;       // Если успешно, то определяем, что уже есть запущенный экземпляр
        } else {
            sharedMemory.create(sizeof(mainWin)); // В противном случае выделяем размером с указатель кусок памяти   xxx1 байт памяти
            is_running = false;                   // И определяем, что других экземпляров не запущено
        }
        semaphore.release(); // Опускаем семафор
        QCommandLineParser parser;
        parser.addPositionalArgument("url", "Url of file to open");
        parser.process(app);
        if (is_running) {
            system("pause");
            if (parser.positionalArguments().length())
                for (const QString& fileName : parser.positionalArguments())
                    instance()->loadFile(fileName);
            return 1;
        } else {
        }
    }

    SplashScreen* splash = new SplashScreen(QPixmap(QStringLiteral(":/256.png")));
    splash->setAttribute(Qt::WA_DeleteOnClose);
    splash->show();

    {
        /*
        Platform        Valid suffixes
        Windows         .dll, .DLL
        Unix/Linux      .so
        AIX             .a
        HP-UX           .sl, .so (HP-UXi)
        macOS and iOS	.dylib, .bundle, .so
        */
#ifdef __unix__
#ifdef QT_DEBUG
        const QString suffix("*.so");
#else
        const QString suffix("*.so");
#endif
#elif _WIN32
        const auto suffix = QStringLiteral("*.dll");
#else
        static_assert(false, "Select OS");
#endif
        // load plugins
        QDir dir(QApplication::applicationDirPath() + "/plugins");
        if (dir.exists()) { // Поиск всех файлов в папке "plugins"
            QStringList listFiles(dir.entryList(QStringList(suffix), QDir::Files));
            for (const auto& str : listFiles) { // Проход по всем файлам
                splash->showMessage(QObject::tr("Load plugin %1\n\n\n").arg(str), Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
                QPluginLoader loader(dir.absolutePath() + "/" + str);
                if (auto* pobj = loader.instance(); pobj) { // Загрузка плагина
                    if (auto* file = qobject_cast<FilePlugin*>(pobj); file) {
                        file->setInfo(loader.metaData().value("MetaData").toObject());
                        App::filePlugins().emplace(file->type(), file);
                        continue;
                    }
                    if (auto* shape = qobject_cast<Shapes::Plugin*>(pobj); shape) {
                        shape->setInfo(loader.metaData().value("MetaData").toObject());
                        App::shapePlugins().emplace(shape->type(), shape);
                        continue;
                    }
                    if (auto* gCode = qobject_cast<GCodePlugin*>(pobj); gCode) {
                        gCode->setInfo(loader.metaData().value("MetaData").toObject());
                        App::gCodePlugins().emplace(gCode->type(), gCode);
                        continue;
                    }
                }
            }
        }

        if (1) { // add dummy gcode plugin
            auto parser = new GCode::Plugin(&app);
            App::filePlugins().emplace(parser->type(), parser);
        }

        QSettings settings;
        settings.beginGroup("MainWindow");
        QString locale(settings.value("locale").toString());
        if (locale.isEmpty())
            locale = QLocale().name().left(2);
        settings.setValue("locale", locale);
        settings.endGroup();
        MainWindow::translate(locale);
    }

    SettingsDialog().accept();

    MainWindow mainWin;
    mainWin.setObjectName("MainWindow");
    mainWin.setIconSize({24, 24});

    QCommandLineParser parser;
    parser.addPositionalArgument("url", "Url of file to open");
    parser.process(app);

    for (const QString& fileName : parser.positionalArguments())
        mainWin.loadFile(fileName);

    mainWin.show();
    splash->finish(&mainWin);

    return app.exec();
}
