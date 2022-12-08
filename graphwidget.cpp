/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.5-0                */
/*                                               */
/*             git version tree browser          */
/*                                               */
/*   28. December 2021                           */
/*                                               */
/*         This program is licensed under        */
/*           GNU GENERAL PUBLIC LICENSE          */
/*            Version 3, 29 June 2007            */
/*                                               */
/* --------------------------------------------- */

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <QtGui>
#include <QRegExp>
#include <QtOpenGL/QGLWidget>
#include <QSettings>
#include <QAction>
#include <QMenu>
#include <QScrollBar>

#include <math.h>

#include <iostream>
#include <string>

#include "execute_cmd.h"
#include "graphwidget.h"
#include "edge.h"
#include "node.h"
#include "mainwindow.h"
#include "fromtoinfo.h"
#include "comparetree.h"
#include "versionadapter.h"
#include "edgeadapter.h"

using namespace std;

string timestamp()
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    stringstream str;

    str << tv.tv_sec << " : " << tv.tv_usec << "  ";
    return str.str();
}

GraphWidget::GraphWidget(MainWindow* _parent)
    : QGraphicsView(_parent),
    toVersion(NULL),
    fromToInfo(NULL),
    rootVersion(NULL),
    localHeadVersion(NULL),
    headVersion(NULL),
    mwin(_parent),
    compareTree(NULL),
    pan(false),
    connectorStyle(0),
    maxLines(0),
    currentLines(0),
    shortHashes(false),
    topDownView(false),
    horizontalSort(0),
    remotes(false),
    xfactor(1),
    yfactor(1),
    commentColumns(-1),
    commentMaxlen(-1),
    selectedVersion(NULL)
{

    if (mwin)
    {
        maxLines = mwin->getMaxLines();
        connectorStyle = mwin->getConnectorStyle();
        shortHashes = mwin->getShortHashes();
        topDownView = mwin->getTopDownView();
        horizontalSort = mwin->getHorizontalSort();
        remotes = mwin->getRemotes();
    }

    // scene
    QGraphicsScene* scene = new QGraphicsScene(this);

    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);

    // try to get hardware acceleration
    QSettings settings;
    QGLWidget* gl = new QGLWidget();

    if (gl->isValid() && mwin->getOpenGLRendering())
    {
        setViewport(gl);
        settings.setValue("openGLRendering", true);
    }
    else
    {
        settings.setValue("openGLRendering", false);
    }

    // view settings
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // create root node
    clear();
}

void GraphWidget::updateFromToInfo()
{
    if (fromToInfo)
        fromToInfo->update();
}

void GraphWidget::test()
{
    QMap<QString, Version*> nodes;

    QStringList nodeNames = (QStringList()
                             << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H"
                             << "I" << "J" << "K" << "L" << "M" << "N" << "O");

    foreach (const QString& n, nodeNames)
    {
        QString line = "#0#0##(tag: " + n + ")#";
        QStringList parts = line.split(QChar('#'));

        nodes[n] = new Version(globalVersionInfo, this);
        nodes[n]->processGitLogInfo(line, parts);

        scene()->addItem(nodes[n]);
    }

    QStringList edgeData = (QStringList()
                            << "OE" << "OF" << "ON" << "EA" << "ED" << "DB" << "DC" << "NG"
                            << "NM" << "MH" << "MI" << "MJ" << "MK" << "ML");

    foreach (const QString& e, edgeData)
    {
        scene()->addItem(new Edge(nodes[QString(e[0])],
                                  nodes[QString(e[1])],
                                  this, false, false, 0));
    }

    scene()->addItem(new Edge(rootVersion, nodes[QString("O")], this, false, true, 0));

    preferencesUpdated();

    normalizeGraph();
    setMinSize();
}

void GraphWidget::fitInView()
{
    QRectF from = mapToScene(viewport()->geometry()).boundingRect();
    QRectF to = scene()->itemsBoundingRect();

    aspectCenter(from, to);
    focusFromTo(from, to);
}

void GraphWidget::mousePressEvent(QMouseEvent* _event)
{
    if (_event->button() == Qt::MiddleButton
        || (
            _event->button() == Qt::LeftButton
            && (_event->modifiers() & Qt::ControlModifier)
           )
       )
    {
        pan = true;
        mpos = _event->pos();
        return;
    }
    else
    {
        pan = false;
    }

    if (_event->button() == Qt::LeftButton)
    {
        Version* sv = NULL;
        int zval = -1;

        QList<QGraphicsItem*> underMouse = items(_event->pos());
        foreach(QGraphicsItem * it, underMouse)
        {
            if (it->type() != QGraphicsItem::UserType + 1)
                continue;

            Version* v = dynamic_cast<Version*>(it);

            if (!v)
                continue;

            if (v->zValue() > zval)
            {
                zval = v->zValue();
                sv = v;
            }
        }

        if (sv && sv->isSelected() == false && sv != rootVersion)
        {
            resetSelection();
            selectedVersion = sv;
            sv->setSelected(true);
            sv->update();
        }
    }

    QGraphicsView::mousePressEvent(_event);
}

void GraphWidget::resetSelection()
{
    if (selectedVersion)
    {
        selectedVersion->setSelected(false);
        selectedVersion->update();
        selectedVersion = NULL;
    }
}

void GraphWidget::mouseMoveEvent(QMouseEvent* _event)
{
    if (pan == true)
    {
        QPoint pt = _event->pos() - mpos;

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - pt.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - pt.y());
        mpos = _event->pos();
    }
    QGraphicsView::mouseMoveEvent(_event);
}

void GraphWidget::mouseReleaseEvent(QMouseEvent* _event)
{
    pan = false;
    QGraphicsView::mouseReleaseEvent(_event);
}

void GraphWidget::wheelEvent(QWheelEvent* event)
{
    pan = false;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    scaleView(pow((double) 1.3, event->delta() / 240.0));
#else
    scaleView(pow((double) 1.3, event->angleDelta().y() / 240.0));
#endif
}

void GraphWidget::keyPressEvent(QKeyEvent* _event)
{
    switch (_event->key())
    {
        case Qt::Key_F:
            if (mwin && Qt::ControlModifier == QApplication::keyboardModifiers())
            {
                mwin->getTagTreeDock()->show();
                mwin->getTagTree()->getSearch()->setFocus();
            }
            break;
        case Qt::Key_W:
            verticalScrollBar()->setSliderPosition(
                verticalScrollBar()->sliderPosition() -
                (Qt::ShiftModifier == QApplication::keyboardModifiers() ? getYFactor() / 2 : 2));
            break;
        case Qt::Key_S:
            verticalScrollBar()->setSliderPosition(
                verticalScrollBar()->sliderPosition() +
                (Qt::ShiftModifier == QApplication::keyboardModifiers() ? getYFactor() / 2 : 2));
            break;
        case Qt::Key_D:
            horizontalScrollBar()->setSliderPosition(
                horizontalScrollBar()->sliderPosition() +
                (Qt::ShiftModifier == QApplication::keyboardModifiers() ? getXFactor() / 2 : 2));
            break;
        case Qt::Key_A:
            horizontalScrollBar()->setSliderPosition(
                horizontalScrollBar()->sliderPosition() -
                (Qt::ShiftModifier == QApplication::keyboardModifiers() ? getXFactor() / 2 : 2));
            break;
        case Qt::Key_Plus:
            zoomIn();
            break;
        case Qt::Key_Minus:
            zoomOut();
            break;
        case Qt::Key_1:
            fitInView();
            break;
        case Qt::Key_O:
            focusElements("HEAD", true);
            break;
        case Qt::Key_H:
            focusCurrent();
            break;
        default:
            QGraphicsView::keyPressEvent(_event);
    }
}

void GraphWidget::drawBackground(QPainter* _painter, const QRectF&)
{
    _painter->setBrush(backgroundColor);
    _painter->drawRect(sceneRect());
}

void GraphWidget::scaleView(qreal scaleFactor)
{
    transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    scale(scaleFactor, scaleFactor);
}

void GraphWidget::zoomIn()
{
    scaleView(qreal(1.05));
}

void GraphWidget::zoomOut()
{
    scaleView(1 / qreal(1.05));
}

void GraphWidget::setGitLogFileConstraint(const QString& _fileConstraint)
{
    // redirected ;(
    mwin->updatePbFileConstraint(_fileConstraint);

    // empty set
    fileConstraintHashes = QSet<QString>();

    // keep file constraint
    fileConstraint = _fileConstraint;

    if (fileConstraint.isEmpty() == false)
    {
        // get a list of all hashes, where _fileConstraint has been touched
        QString cmd = "git -C "
            + localRepositoryPath
            + " log --pretty=\""
            + (shortHashes ? "%h" : "%H")
            + "\"";

        if (remotes)
            cmd += " --remotes";

        if (mwin->getSelectedBranch().size())
            cmd += " " + mwin->getSelectedBranch();

        if (fileConstraint.size())
            cmd += " -- " + fileConstraint;

        //
        QList<QString> cache;
        execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());
        foreach(QString it, cache)
        {
            fileConstraintHashes << it.trimmed();
        }
    }

    // set fileConstraint flag
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* v = dynamic_cast<Version*>(it);

        if (!v)
            continue;

        v->clearFileConstraintEdgeList();
        bool stat = v->getHash().size() && fileConstraintHashes.contains(v->getHash());

        v->setFileConstraint(stat);
    }

    // apply new constraint
    if (fileConstraint.isEmpty() == false)
    {
        rootVersion->reduceToFileConstraint(rootVersion);
    }

    //
    update();
}

Version* GraphWidget::gitlogSingle(QString _hash, bool _create)
{
    if (_hash.size() && _create == false)
        return getVersionByHash(_hash);

    if (localRepositoryPath.isEmpty())
        return NULL;

    QString cmd = "git -C "
        + localRepositoryPath
        + " log --graph -1 --pretty=\"#"
        + (shortHashes ? "%h" : "%H")
        + "#%at#%an#%d#%s#\"";

    if (_hash.isEmpty() == false)
        cmd += " " + _hash;
    else if (mwin->getSelectedBranch().size())
        cmd += " " + mwin->getSelectedBranch();

    QList<QString> cache;

    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    QString line = cache.front();

    QStringList parts = line.split(QChar('#'));

    // abort, if too short...
    if (parts.size() < 6)
    {
        cerr << "Error: Input too short " << line.toUtf8().data() << endl;
        return NULL;
    }

    // check if the Version object already exists
    QString hash = parts.at(1);

    Version* v = (_create == false) ? getVersionByHash(hash) : NULL;

    if (!v)
    {
        // create an object...
        v = new Version(globalVersionInfo, this);

        // if the key information has already been parsed, use it
        v->setKeyInformation(keyInformationCache.value(hash, QMap<QString, QStringList>()));

        // init or update
        if (v->processGitLogInfo(line, parts))
        {
            keyInformationCache[hash] = v->getKeyInformation();
        }

        mwin->getTagTree()->blockSignals(true);
        mwin->getTagTree()->addData(v);
        mwin->getTagTree()->compress();
        mwin->getTagTree()->blockSignals(false);
    }

    return v;
}

void GraphWidget::gitlog(bool _changed)
{
    if (_changed)
    {
        keyInformationCache.clear();
        resetSelection();
    }

    setUpdatesEnabled(false);

    saveImportantVersions();

    clear();

    QString cmd = "git -C "
        + localRepositoryPath
        + " log --graph --pretty=\"#"
        + (shortHashes ? "%h" : "%H")
        + "#%at#%an#%d#%s#\"";

    if (remotes)
        cmd += " --remotes";

    if (mwin->getSelectedBranch().size())
        cmd += " " + mwin->getSelectedBranch();

    QList<QString> cache;

    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());
    process(cache);

    adjustComments();

    restoreImportantVersions();
    setUpdatesEnabled(true);
}

void GraphWidget::setCompareTree(CompareTree* _tree)
{
    compareTree = _tree;
}

void GraphWidget::removeFilter()
{
    setGitLogFileConstraint();
}

const MainWindow* GraphWidget::getMainWindow() const
{
    return mwin;
}

void GraphWidget::load(const QString& _path)
{
    setUpdatesEnabled(false);

    QFile file(_path);
    QList<QString> cache;

    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        char buffer[65536];
        while (!file.atEnd())
        {
            file.readLine(buffer, 65535);
            cache.push_back(QString(buffer));
        }
        file.close();
    }
    process(cache);

    setUpdatesEnabled(true);
}

void GraphWidget::debugExit(char _c,
                            int _column,
                            int _lineNumber,
                            const QString& _tree,
                            const QString& _previousTree,
                            const QString& _line)
{
    std::cerr << "Unknown sequence: current [" << _c
              << "] line [" << _lineNumber << "] column [" << _column << "]" << std::endl;
    std::cerr << _line.toUtf8().data() << std::endl;
    std::cerr << std::endl;
    std::cerr << _tree.toUtf8().data() << std::endl;
    std::cerr << _previousTree.toUtf8().data() << std::endl;
    std::cerr << std::endl;
    exit(0);
}

void GraphWidget::debugGraphParser(
    const QString& _tree,
    const QVector<Version*>& _slots)
{
    std::cout << _tree.toUtf8().data() << std::endl;
    foreach(const Version * v, _slots)
    {
        std::cout << v << "  ";
    }
    std::cout << std::endl;
}

void GraphWidget::process(QList<QString> _cache)
{
    // cerr << "process start " << timestamp() << endl;

    // reset local head
    headVersion = NULL;

    connectorStyle = mwin->getConnectorStyle();

    clear();

    mwin->getTagTree()->blockSignals(true);
    mwin->getTagTree()->resetTagTree();

    int linecount = 0;
    int maxTreePatternLength = 0;

    QVector<Version*> branchslots;
    QVector<Version*> previousBranchslots;

    // branchslots
    // 0        : rootnode
    // i%2 == 0 : version node
    //            * version
    //            |

    QList<QString> swap_cache;

    foreach (const QString& line, _cache)
    {
        swap_cache.push_front(line);
        linecount++;
        if (linecount > maxLines)
            break;
    }

    // now root node is in the first line...
    currentLines = linecount;

    QString previousTree;
    QRegExp treePattern("^([*\\\\/\\. |\\-_]*[*\\\\/\\.|\\-_]+)");

    int linenumber = 0;

    foreach (const QString& line, swap_cache)
    {
        // get the tree pattern
        int pos = treePattern.indexIn(line, 0);

        if (pos == -1)
        {
            std::cerr << "No --graph pattern contained in line " << linenumber << " : " << line.toUtf8().data() << std::endl;
            continue;
        }

        linenumber++;

        // get --graph tree pattern
        int len = treePattern.matchedLength();
        QString tree = treePattern.cap(1);

        // allocate space
        if (previousTree.isEmpty())
        {
            branchslots = QVector<Version*>(tree.size());
            maxTreePatternLength = len;
        }
        else if (maxTreePatternLength < len)
        {
            branchslots.resize(len + 1);
            maxTreePatternLength = len;
        }

        // shift versions:
        // use previousTree and tree to shift the versions
        // stored in previousBranchslots to branchslots

        // position of new version '*'
        int newVersion = -1;

        for (int i = 0; i < tree.size(); i++)
        {
            // current characters
            // cll cl cm cr
            // pll pl pm pr
            char cll = (i > 1) ? tree[i - 2].toLatin1() : 0;
            // char cl = (i > 0) ? tree[i - 1].toLatin1() : 0;
            char cm = tree[i].toLatin1();
            // char cr = (tree.size() >= i) ? tree[i + 1].toLatin1() : 0;
            char pll = (i > 1 && previousTree.size() > i - 2) ? previousTree[i - 2].toLatin1() : 0;
            char pl = ((previousTree.size() >= i) && (i > 0)) ? previousTree[i - 1].toLatin1() : 0;
            char pm = (previousTree.size() > i) ? previousTree[i].toLatin1() : 0;
            char pr = (previousTree.size() > i + 1) ? previousTree[i + 1].toLatin1() : 0;

            // hope' all cases are covered
            switch (cm)
            {
                case '|':
                    if (pm == '|' || pm == '*' || pm == '/' || pm == '\\')
                        branchslots[i] = previousBranchslots[i];
                    else if (pl == '/')
                        branchslots[i] = previousBranchslots[i - 1];
                    else if (pr == '\\')
                        branchslots[i] = previousBranchslots[i + 1];
                    // else debugExit(cm,i,linenumber,tree,previousTree,line);
                    break;
                case '/':
                    if (cll == '_')
                        branchslots[i] = branchslots[i - 2];
                    else if (pll == '/')
                        branchslots[i] = previousBranchslots[i - 2];
                    else if (pl == '|' || pl == '*' || pl == '/')
                        branchslots[i] = previousBranchslots[i - 1];
                    else if (pm == '\\' || pm == '|')
                        branchslots[i] = previousBranchslots[i];
                    // else debugExit(cm,i,linenumber,tree,previousTree,line);
                    break;
                case '_':
                    if (cll == '_')
                        branchslots[i] = branchslots[i - 2];
                    else if (pll == '/')
                        branchslots[i] = previousBranchslots[i - 2];
                    // else debugExit(cm,i,linenumber,tree,previousTree,line);
                    break;
                case '\\':
                    if (pm == '/')
                        branchslots[i] = previousBranchslots[i];
                    else if (pr == '|' || pr == '*' || pr == '\\')
                        branchslots[i] = previousBranchslots[i + 1];
                    // else debugExit(cm,i,linenumber,tree,previousTree,line);
                    break;
                case '.':
                case '-':
                    if (pr == '\\' || pr == ' ')
                        branchslots[i] = previousBranchslots[i + 1];
                    // else debugExit(cm,i,linenumber,tree,previousTree,line);
                    break;
                case '*':
                    newVersion = i;
                    break;
                case ' ':
                    break;
                default:
                    std::cerr << "Character " << cm << " not recognized." << std::endl;
                    exit(0);
                    break;
            }
        }

        if (newVersion != -1)
        {
            // in each line at a maximum one new version '*' is contained
            // this version has got one parent and n merge sources
            Version* parent = NULL;
            QList<Version*> mergeSources;

            int i = newVersion;
            // char cll = (i > 1) ? tree[i - 2].toLatin1() : 0;
            // char cl = (i > 0) ? tree[i - 1].toLatin1() : 0;
            // char cm = tree[i].toLatin1();
            char cr = (tree.size() >= i) ? tree[i + 1].toLatin1() : 0;
            // char pll = (i > 1 && previousTree.size() > i - 2) ? previousTree[i - 2].toLatin1() : 0;
            char pl = ((previousTree.size() >= i) && (i > 0)) ? previousTree[i - 1].toLatin1() : 0;
            char pm = (previousTree.size() > i) ? previousTree[i].toLatin1() : 0;
            char pr = (previousTree.size() > i + 1) ? previousTree[i + 1].toLatin1() : 0;

            if (pl == '/')
                parent = previousBranchslots[i - 1];
            else if (pm == '*' || pm == '|')
                parent = previousBranchslots[i];
            else if (pl == '\\')
                parent = previousBranchslots[i + 1];

            if (parent == NULL)
                parent = rootVersion;

            // collect merge sources
            if (pl == '/' && parent != previousBranchslots[i - 1])
                mergeSources.push_back(previousBranchslots[i - 1]);
            if ((pm == '|' || pm == '*') && parent != previousBranchslots[i])
                mergeSources.push_back(previousBranchslots[i]);
            if (pr == '\\' && parent != previousBranchslots[i + 1])
                mergeSources.push_back(previousBranchslots[i + 1]);
            if (cr == '-')
            {
                int j = i + 1;
                while (j < tree.size() && (tree[j] == '-' || tree[j] == '.'))
                {
                    Version* tmp = branchslots[j];
                    if (tmp != NULL && tmp != parent)
                        mergeSources.push_back(tmp);
                    j++;
                }
            }

            // starting at pos+len the version information is contained
            QString info = line.mid(pos + len);

            // tokenize
            QStringList parts = info.split(QChar('#'));

            // abort, if too short...
            if (parts.size() < 6)
            {
                cerr << "Error: Input too short " << line.toUtf8().data() << endl;
                break;
            }

            // create version node
            Version* v = new Version(globalVersionInfo, this);

            // if the key information has already been parsed, use it
            QString hash = parts.at(1);
            v->setKeyInformation(keyInformationCache.value(hash, QMap<QString, QStringList>()));

            // init or update
            if (v->processGitLogInfo(info, parts))
            {
                keyInformationCache[hash] = v->getKeyInformation();
            }

            //
            v->setIsFoldable(mwin->getVersionIsFoldable(v->getKeyInformation()));

            mwin->getTagTree()->addData(v);

            // main?
            v->setIsMain(i == 0);
            scene()->addItem(v);

            Edge* e = new Edge (parent, v, this, false, parent == rootVersion);
            scene()->addItem(e);

            foreach(Version * merge, mergeSources)
            {
                Edge* mergeArrow = new Edge(merge, v, this, true, false);

                scene()->addItem(mergeArrow);
            }

            // replace parent
            branchslots[i] = v;
            // The last line contains the first version
            // of the git log output.
            headVersion = v;
        }

        //debugGraphParser(tree, branchslots);
        previousTree = tree;
        previousBranchslots = branchslots;
        for (int i = 0; i < branchslots.size(); i++)
        {
            branchslots[i] = NULL;
        }
    }

    localHeadVersion = gitlogSingle();

    rootVersion->collectFolderVersions(rootVersion, NULL);
    normalizeGraph();
    setMinSize();

    mwin->getTagTree()->compress();
    mwin->getTagTree()->blockSignals(false);

    // cerr << "process end " << timestamp() << endl;
}

void GraphWidget::clear()
{
    if (rootVersion)
        scene()->removeItem(rootVersion);

    foreach(QGraphicsItem * it, scene()->items())
    {
        scene()->removeItem(it);
    }

    rootVersion = new Version(this);
    rootVersion->setPos(0, 0);
    scene()->addItem(rootVersion);

    fromToInfo = new FromToInfo(this);
    fromToInfo->hide();
    scene()->addItem(fromToInfo);
}

void GraphWidget::commitInfo(const Version* _v, QTextEdit* _tedi)
{
    // get data
    QString cmd = "git -C " + localRepositoryPath + " log -1 " + _v->getHash();

    _tedi->clear();
    QList<QString> cache;

    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());
    foreach(const QString& str, cache)
    {
        _tedi->insertPlainText(str);
    }
    _tedi->moveCursor(QTextCursor::Start);
}

void GraphWidget::fillCompareWidgetFromToInfo()
{
    // from version part
    mwin->getFromComboBox()->clear();
    mwin->getCompareTreeFromTextEdit()->clear();
    foreach(Version * it, fromVersions)
    {
        it->setMatched(true);
        mwin->getFromComboBox()->addItem(it->getCommitDateString(), QVariant::fromValue(VersionPointer(it)));
    }
    mwin->getCompareTreeFromPushButton()->setEnabled(fromVersions.size() > 0);

    // to version part
    mwin->getCompareTreeToTextEdit()->clear();
    if (toVersion)
    {
        toVersion->setMatched(true);
        mwin->getToDateLabel()->setText(toVersion->getCommitDateString());
        mwin->getCompareTreeToPushButton()->setEnabled(true);
        commitInfo(toVersion, mwin->getCompareTreeToTextEdit());
    }
    else
    {
        mwin->getToDateLabel()->setText(QString());
        mwin->getCompareTreeToPushButton()->setEnabled(false);
    }
}

void GraphWidget::diffStagedChanges()
{
    resetMatches();

    fromVersions = QSet<Version*>();
    fromVersions.insert(localHeadVersion);
    toVersion = NULL;

    fillCompareWidgetFromToInfo();
    mwin->getToDateLabel()->setText(QString("Staged Changes"));

    compareTree->viewLocalChanges(true);

    // Force visibility of compare files window
    mwin->getCompareTreeDock()->show();

    // cursor
    fromToInfo->setFromToPosition(fromVersions, NULL);
    fromToInfo->show();
}

void GraphWidget::viewThisVersion(Version* _v)
{
    resetMatches();

    fromVersions = QSet<Version*>();
    fromVersions.insert(_v);
    toVersion = NULL;

    fillCompareWidgetFromToInfo();
    mwin->getToDateLabel()->setText(QString("root Version"));

    compareTree->viewThisVersion(_v->getHash());

    // ensure visibility of Compare Versions dock
    mwin->getCompareTreeDock()->show();

    // cursor
    fromToInfo->setFromToPosition(fromVersions, NULL);
    fromToInfo->show();
}

void GraphWidget::diffLocalChanges()
{
    resetMatches();

    fromVersions = QSet<Version*>();
    fromVersions.insert(localHeadVersion);
    toVersion = NULL;

    fillCompareWidgetFromToInfo();
    mwin->getToDateLabel()->setText(QString("Local Changes"));

    compareTree->viewLocalChanges(false);

    // Force visibility of compare files window
    mwin->getCompareTreeDock()->show();

    // cursor
    fromToInfo->setFromToPosition(fromVersions, NULL);
    fromToInfo->show();
}

void GraphWidget::compareVersions(Version* _v1, Version* _v2)
{
    resetMatches();

    fromVersions = QSet<Version*>();
    fromVersions.insert(_v1);
    toVersion = _v2;

    fillCompareWidgetFromToInfo();

    QStringList fromVersionHashes(_v1->getHash());

    compareTree->compareHashes(fromVersionHashes, _v2->getHash());

    // ensure visibility of Compare Versions dock
    mwin->getCompareTreeDock()->show();

    // cursor
    fromToInfo->setFromToPosition(fromVersions, _v2);
    fromToInfo->show();
}

void GraphWidget::compareToSelected(Version* _v)
{
    compareVersions(selectedVersion, _v);
}

void GraphWidget::compareToLocalHead(Version* _v)
{
    compareVersions(_v, localHeadVersion);
}

void GraphWidget::compareToBranchBaseline(Version* _v)
{
    Version* baseline = _v->lookupBranchBaseline();

    if (baseline)
        compareVersions(baseline, _v);
}

void GraphWidget::compareToPrevious(Version* _v)
{
    // more complex because multiple predecessors are possible
    resetMatches();

    _v->getPredecessors(fromVersions);
    toVersion = _v;

    fillCompareWidgetFromToInfo();

    // compare to previous...
    QStringList fromVersionHashes;

    _v->getPredecessorHashes(fromVersionHashes);
    compareTree->compareHashes(fromVersionHashes, _v->getHash());

    // ensure visibility of Compare Versions dock
    mwin->getCompareTreeDock()->show();

    // view cursor
    fromToInfo->setFromToPosition(fromVersions, _v);
    fromToInfo->show();
}

void GraphWidget::focusCurrent()
{
    if (localHeadVersion)
    {
        resetMatches();
        localHeadVersion->setMatched(true);
        displayHits(localHeadVersion);
    }
}

void GraphWidget::focusFromVersion()
{
    int idx = mwin->getFromComboBox()->currentIndex();

    Version* v = mwin->getFromComboBox()->itemData(idx).value<VersionPointer>();

    if (v)
    {
        resetMatches();
        v->setMatched(true);
        if (v->ensureUnfolded())
        {
            updateGraphFolding();
        }
        displayHits(v);
    }
}

void GraphWidget::focusToVersion()
{
    if (toVersion)
    {
        resetMatches();
        toVersion->setMatched(true);
        if (toVersion->ensureUnfolded())
        {
            updateGraphFolding();
        }
        displayHits(toVersion);
    }
}

void GraphWidget::setMinSize(bool _resize)
{
    QRectF r = scene()->itemsBoundingRect().adjusted(-100, -100, 100, 100);

    scene()->setSceneRect(r);
    if (_resize)
        QGraphicsView::fitInView(r, Qt::KeepAspectRatio);
    else
        viewport()->repaint();
}

void GraphWidget::forceUpdate()
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() == QGraphicsItem::UserType + 1)
        {
            Version* n = dynamic_cast<Version*>(it);
            if (n)
                n->calculateLocalBoundingBox();
        }
        it->update();
    }
    viewport()->repaint();
}

void GraphWidget::calculateGraphicsViewPosition()
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* n = dynamic_cast<Version*>(it);

        if (n)
        {
            n->calculateCoordinates(getXFactor(), getYFactor());
            n->calculateLocalBoundingBox();
        }
    }
}

void GraphWidget::normalizeGraph()
{
    setBlockItemChanged(true);

    int sort = mwin->getHorizontalSort();

    if (sort == 1 || sort == 2)
    {
        rootVersion->calculateWeightRecurse();
    }

    if (sort)
    {
        rootVersion->applyHorizontalSort(sort);
    }

    // the following 5 commands create a collision free tree graph
    rootVersion->simpleTreeGeometry(NULL);
    rootVersion->centerParents(NULL);
    rootVersion->shiftTree();
    rootVersion->addShift(0);

    // now the QGraphicsView geometry is calculated
    calculateGraphicsViewPosition();

    // head on top or bottom
    if (topDownView)
        flipY();

    // set parent versions recurse
    rootVersion->linkTreenodes(NULL);
    adjustAllEdges();

    // update the from-to version info cursor
    if (fromToInfo)
        fromToInfo->update();

    setBlockItemChanged(false);
}

void GraphWidget::adjustComments()
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* v = dynamic_cast<Version*>(it);

        if (v)
            v->updateCommentInformation(commentColumns, commentMaxlen);
    }
}

void GraphWidget::adjustAllEdges()
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 2)
            continue;

        Edge* e = dynamic_cast<Edge*>(it);

        if (e)
            e->adjust();
    }
}

void GraphWidget::setBlockItemChanged(bool _val)
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* v = dynamic_cast<Version*>(it);

        if (v)
            v->setBlockItemChanged(_val);
    }
}

Version* GraphWidget::findVersion(const QString& _hash)
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* v = dynamic_cast<Version*>(it);

        if (v && v->getHash() == _hash)
            return v;
    }
    return NULL;
}

void GraphWidget::updateGraphFolding(Version* _v)
{
    if (_v && _v->isFolder())
    {
        QList<Version*> versions;

        if (mwin->getAnimated())
        {
            versions.push_back(_v->isFolded() ? _v : _v->getFolderVersions().front());
            displayHits(versions, false);
        }

        _v->foldAction();
        normalizeGraph();
        setMinSize(false);

        versions.clear();
        versions.push_back(_v);
        displayHits(versions, false);
    }
    else
    {
        normalizeGraph();
        setMinSize(false);
    }
}

void GraphWidget::resetMatches()
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* v = dynamic_cast<Version*>(it);

        if (v)
            v->setMatched(false);
    }
}

int GraphWidget::matchVersions(const QString& _text, QList<Version*>& _matches, bool _exactMatch)
{
    _matches.clear();

    if (!_text.isEmpty())
    {
        QRegExp pattern(_text);

        foreach(QGraphicsItem * it, scene()->items())
        {
            if (it->type() != QGraphicsItem::UserType + 1)
                continue;

            Version* n = dynamic_cast<Version*>(it);

            if (n && n->findMatch(pattern, _text, _exactMatch))
            {
                _matches.push_back(n);
            }
        }
    }

    return _matches.size();
}

bool GraphWidget::focusElements(const QString& _text, bool _exactMatch)
{
    pan = false;
    resetMatches();

    QList<Version*> matches;

    matchVersions(_text, matches, _exactMatch);

    displayHits(matches);

    return matches.size() > 0;
}

void GraphWidget::getMarkedupVersions(QList<Version*>& _markup, bool _selected)
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* v = dynamic_cast<Version*>(it);

        if (v && (v->getMatched() || (_selected && v->isSelected())))
            _markup.push_back(v);
    }
}

bool GraphWidget::focusElements(const QList<Version*>& _markup)
{
    foreach(Version * v, _markup)
    {
        v->setMatched(true);
        v->ensureUnfolded();
    }
    displayHits(_markup);

    return _markup.size() > 0;
}

void GraphWidget::displayHits(Version* _v)
{
    QList<Version*> tmp;

    tmp.push_back(_v);
    displayHits(tmp);
}

void GraphWidget::displayHits(const QList<Version*>& _hits, bool _unfold)
{
    QRectF from = mapToScene(viewport()->geometry()).boundingRect();
    QRectF to;

    if (_hits.size() > 0)
    {
        // ensure visibility (TODO update only if folding has changed)
        if (_unfold)
            updateGraphFolding();

        QGraphicsItem* it = _hits.front();
        QRectF tmp = QRectF(-10, -10, 20, 20).translated(it->scenePos());

        foreach(QGraphicsItem * it, _hits)
        {
            tmp |= QRectF(-10, -10, 20, 20).translated(it->scenePos());
        }

        if (mwin->getIncludeSelected() && selectedVersion)
        {
            tmp |= QRectF(-10, -10, 20, 20).translated(selectedVersion->scenePos());
        }

        tmp.adjust(-getXFactor(), -getYFactor(), getXFactor(), getYFactor());
        to = tmp;
    }
    else
    {
        if (mwin->getIncludeSelected() && selectedVersion)
        {
            to = QRectF(-10, -10, 20, 20).translated(selectedVersion->scenePos());
            to.adjust(-getXFactor(), -getYFactor(), getXFactor(), getYFactor());
        }
        else
        {
            to = scene()->itemsBoundingRect();
        }
    }

    // expand to rectangle to have the same aspect ratio than from
    aspectCenter(from, to);

    // focus including animation if set
    focusFromTo(from, to);
}

void GraphWidget::focusFromTo(const QRectF& _from, const QRectF& _to)
{
    // check if animation does make sense...
    QGraphicsView::fitInView(_to);
    QRectF comp = mapToScene(viewport()->geometry()).boundingRect();

    qreal fx, fy, fw, fh;
    qreal tx, ty, tw, th;

    _from.getRect(&fx, &fy, &fw, &fh);
    comp.getRect(&tx, &ty, &tw, &th);

    qreal delta = fabs(fx - tx) + fabs(fy - ty) + fabs(fw - tw) + fabs(fh - th);

    // from is different to to
    if (delta > 25)
    {
        QGraphicsView::fitInView(_from);
        if (mwin->getAnimated())
            animatedFocus(_from, _to);

        QGraphicsView::fitInView(_to);
    }
    viewport()->repaint();
}

void GraphWidget::animatedFocus(const QRectF& _from, const QRectF& _to)
{
    QElapsedTimer timer;

    timer.start();
    QGraphicsView::fitInView(animatedFocus(_from, _to, 0.0));
    viewport()->repaint();
    qint64 elapsed = timer.nsecsElapsed() + 1; // ensure > 0

    timer.invalidate();

    qint64 frames = 1000000000 / elapsed;
    double add = 1.0 / (frames + 1);

    for (double morph = add; morph < 1.0; morph += add)
    {
        QGraphicsView::fitInView(animatedFocus(_from, _to, morph));
        viewport()->repaint();
    }
}

void GraphWidget::aspectCenter(QRectF& _from, QRectF& _to)
{
    // give _to the same aspect than _from.
    // old _to is centered in the new one

    double rf = _from.width() / _from.height();
    double rt = _to.width() / _to.height();

    if (rf > rt)
    {
        double w = rf * _to.height();
        _to = QRectF(_to.left() - 0.5 * (w - _to.width()),
                     _to.top(),
                     w,
                     _to.height());
    }
    else
    {
        double h = _to.width() * _from.height() / _from.width();
        _to = QRectF(_to.left(),
                     _to.top() - 0.5 * (h - _to.height()),
                     _to.width(),
                     h);
    }
}

QRectF GraphWidget::animatedFocus(
    const QRectF& _from,
    const QRectF& _to,
    double _morph)
{
    return QRectF (_from.left() + _morph * (_to.left() - _from.left()),
                   _from.top() + _morph * (_to.top() - _from.top()),
                   _from.width() + _morph * (_to.width() - _from.width()),
                   _from.height() + _morph * (_to.height() - _from.height()));
}

void GraphWidget::flipY()
{
    int max = 0;

    foreach(QGraphicsItem * it, scene()->items())
    {
        if ((it->type() == QGraphicsItem::UserType + 1)
            && (it->pos().y() > max))
            max = it->pos().y();
    }
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() == QGraphicsItem::UserType + 1)
        {
            QPointF np(it->pos().x(), max - it->pos().y());
            it->setPos(np);
        }
    }
}

void GraphWidget::setGlobalVersionInfo(const QString& _item, bool _value)
{
    if (_value)
    {
        globalVersionInfo << _item;
    }
    else
    {
        globalVersionInfo.removeOne(_item);
    }
    QGraphicsView::update();
}

void GraphWidget::setLocalRepositoryPath(const QString& _dir)
{
    localRepositoryPath = _dir;
}

const QString& GraphWidget::getLocalRepositoryPath() const
{
    return localRepositoryPath;
}

void GraphWidget::preferencesUpdated(bool _forceUpdate)
{
    bool updateAll = mwin->getXYFactor(xfactor, yfactor);

    if (maxLines != mwin->getMaxLines())
    {
        if (currentLines > 0 && currentLines < maxLines && currentLines < mwin->getMaxLines())
        {
            // no update
        }
        else
        {
            maxLines = mwin->getMaxLines();
            updateAll = true;
        }
    }

    if (shortHashes != mwin->getShortHashes())
    {
        shortHashes = mwin->getShortHashes();
        updateAll = true;
    }

    if (topDownView != mwin->getTopDownView())
    {
        topDownView = mwin->getTopDownView();
        updateAll = true;
    }

    if (horizontalSort != mwin->getHorizontalSort())
    {
        horizontalSort = mwin->getHorizontalSort();
        updateAll = true;
    }

    if (remotes != mwin->getRemotes())
    {
        remotes = mwin->getRemotes();
        updateAll = true;
    }

    int columns, maxlen;

    mwin->getCommentProperties(columns, maxlen);
    if (commentColumns != columns || commentMaxlen != maxlen)
    {
        commentColumns = columns;
        commentMaxlen = maxlen;
        adjustComments();
    }

    if (connectorStyle != mwin->getConnectorStyle())
    {
        connectorStyle = mwin->getConnectorStyle();
        adjustAllEdges();
    }

    if (_forceUpdate || (updateAll && currentLines > 0))
    {
        gitlog();
    }

    updateColors();
}

void GraphWidget::updateColors()
{
    backgroundColor = mwin->getPreferencesColor("colorBackground");
    fromToColor = mwin->getPreferencesColor("colorFromTo");
    nodeColor = mwin->getPreferencesColor("colorVersion");
    selectedColor = mwin->getPreferencesColor("colorSelected");
    searchColor = mwin->getPreferencesColor("colorSearch");
    edgeColor = mwin->getPreferencesColor("colorEdge");
    mergeColor = mwin->getPreferencesColor("colorMerge");
    foldedColor = mwin->getPreferencesColor("colorFolded");
    unfoldedColor = mwin->getPreferencesColor("colorUnfolded");
    fileConstraintColor = mwin->getPreferencesColor("colorFileConstraint");
    forceUpdate();
}

void GraphWidget::foldAll()
{
    rootVersion->foldRecurse(true);
    updateGraphFolding();
    setMinSize();
}

void GraphWidget::unfoldAll()
{
    rootVersion->foldRecurse(false);
    updateGraphFolding();
    setMinSize();
}

void GraphWidget::contextMenuEvent(QContextMenuEvent* _event)
{
    QList<QGraphicsItem*> underCursor = items(_event->pos());

    QGraphicsItem* it = NULL;
    qreal zval = -1;

    foreach(QGraphicsItem * uit, underCursor)
    {
        if (uit->type() == QGraphicsItem::UserType + 1)
        {
            Version* v = dynamic_cast<Version*>(uit);
            if (v && v->isFolder() && v->isFolded())
            {
                it = uit;
                break;
            }
            else if (v && !v->isFolder())
            {
                it = uit;
                break;
            }
            else if (v && !v->isFolded())
            {
              // tricky, if unfolded, the node itself must be matched.
              // check if inside ... path.addEllipse(-15, -15, 30, 30);
              QPointF mpos = mapToScene(_event->pos());
              QPointF gpos = v->scenePos();
              if (QLineF(mpos, gpos).length() <= 15.0)
              {
                it = uit;
                break;
              }
              else if (!it)
              {
                it = uit;
              }
            }
            else if (!it || it->type() != QGraphicsItem::UserType + 1)
            {
                it = uit;
            }
        }
        else if (uit->type() == QGraphicsItem::UserType + 2 && uit == NULL)
        {
            it = uit;
        }
        else if ((uit != rootVersion)
                 && (uit->type() != QGraphicsItem::UserType + 4)
                 && (uit->zValue() > zval))
        {
            it = uit;
            zval = uit->zValue();
        }
    }

    if (it && it->type() == QGraphicsItem::UserType + 1 && it != rootVersion)
    {
        Version* v = dynamic_cast<Version*>(it);

        if (v)
        {
            VersionAdapter adapter(v);

            QMenu menu(&adapter);
            QAction* action = NULL;
            if (v->isFolder())
            {
                action = menu.addAction(QString("Fold/Unfold"));
                connect(action, SIGNAL(triggered()), &adapter, SLOT(foldAction()));
                connect(&adapter, SIGNAL(foldSignal(Version*)), this, SLOT(updateGraphFolding(Version*)));
                menu.addSeparator();
            }
            if (selectedVersion && (v != selectedVersion))
            {
                action = menu.addAction(QString("Compare to selected"));
                connect(action, SIGNAL(triggered()), &adapter, SLOT(compareToSelected()));
            }
            action = menu.addAction(QString("Compare to previous"));
            connect(action, SIGNAL(triggered()), &adapter, SLOT(compareToPrevious()));
            if (v != localHeadVersion)
            {
                action = menu.addAction(QString("Compare to local HEAD"));
                connect(action, SIGNAL(triggered()), &adapter, SLOT(compareToLocalHead()));
            }
            Version* branchBaseline = v->lookupBranchBaseline();
            if (branchBaseline && v != branchBaseline)
            {
                action = menu.addAction(QString("Compare to branch baseline"));
                connect(action, SIGNAL(triggered()), &adapter, SLOT(compareToBranchBaseline()));
            }
            action = menu.addAction(QString("View this version"));
            connect(action, SIGNAL(triggered()), &adapter, SLOT(viewThisVersion()));
            menu.addSeparator();
            // Experimental TODO markup that a subtree exists
            if (!v->isLeaf())
            {
                if (v->getSubtreeHidden())
                {
                    action = menu.addAction(QString("Show subtree"));
                    connect(action, SIGNAL(triggered()), &adapter, SLOT(showSubtree()));
                }
                else
                {
                    action = menu.addAction(QString("Hide subtree"));
                    connect(action, SIGNAL(triggered()), &adapter, SLOT(hideSubtree()));
                }
                menu.addSeparator();
            }

            action = menu.addAction(QString("Focus neighbours"));
            connect(action, SIGNAL(triggered()), &adapter, SLOT(focusNeighbourBox()));
            menu.exec(_event->globalPos());
        }
        return;
    }

    if (it && it->type() == QGraphicsItem::UserType + 2)
    {
        Edge* e = dynamic_cast<Edge*>(it);

        if (e && !e->getInfo())
        {
            EdgeAdapter adapter(e);
            QMenu menu(&adapter);
            QAction* action = NULL;
            action = menu.addAction(QString("Compare adjacent versions"));
            connect(action, SIGNAL(triggered()), &adapter, SLOT(compareVersions()));
            menu.addSeparator();
            action = menu.addAction(QString("Focus neighbours"));
            connect(action, SIGNAL(triggered()), &adapter, SLOT(focusNeighbourBox()));
            action = menu.addAction(QString("Focus source"));
            connect(action, SIGNAL(triggered()), &adapter, SLOT(focusSource()));
            action = menu.addAction(QString("Focus destination"));
            connect(action, SIGNAL(triggered()), &adapter, SLOT(focusDestination()));
            menu.exec(_event->globalPos());
        }
        return;
    }

    if (it && it->type() == QGraphicsItem::UserType + 3)
    {
        QMenu menu(this);
        QAction* action = menu.addAction(QString("Remove Filter"));
        connect(action, SIGNAL(triggered()), this, SLOT(removeFilter()));
        menu.exec(_event->globalPos());
        return;
    }

    QMenu menu(this);
    QAction* action = NULL;

    action = menu.addAction(QString("Focus local HEAD"));
    connect(action, SIGNAL(triggered()), this, SLOT(focusCurrent()));
    action = menu.addAction(QString("Fit in view"));
    connect(action, SIGNAL(triggered()), this, SLOT(fitInView()));
    action = menu.addAction(QString("Reload repository"));
    connect(action, SIGNAL(triggered()), mwin, SLOT(reloadCurrentRepository()));
    menu.addSeparator();
    action = menu.addAction(QString("Reset diff"));
    connect(action, SIGNAL(triggered()), this, SLOT(resetDiff()));
    action = menu.addAction(QString("Diff local changes"));
    connect(action, SIGNAL(triggered()), this, SLOT(diffLocalChanges()));
    action = menu.addAction(QString("Diff staged changes"));
    connect(action, SIGNAL(triggered()), this, SLOT(diffStagedChanges()));
    menu.addSeparator();
    action = menu.addAction(QString("Fold all"));
    connect(action, SIGNAL(triggered()), this, SLOT(foldAll()));
    action = menu.addAction(QString("Unfold all"));
    connect(action, SIGNAL(triggered()), this, SLOT(unfoldAll()));

    if (selectedVersion)
    {
        menu.addSeparator();
        action = menu.addAction(QString("Reset selection"));
        connect(action, SIGNAL(triggered()), this, SLOT(resetSelection()));
    }

    menu.exec(_event->globalPos());
}

QStringList GraphWidget::getFromHashes() const
{
    QStringList result;

    foreach(Version * it, fromVersions)
    {
        result.push_back(it->getHash());
    }
    return result;
}

QString GraphWidget::getToHash() const
{
    return toVersion ? toVersion->getHash() : QString();
}

const QString& GraphWidget::getFileConstraint() const
{
    return fileConstraint;
}

const QSet<Version*>& GraphWidget::getPredecessors() const
{
    return fromVersions;
}

Version* GraphWidget::getVersionByHash(const QString& _hash)
{
    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* v = dynamic_cast<Version*>(it);

        if (v && v->getHash() == _hash)
            return v;
    }
    return NULL;
}

Version* GraphWidget::getSelectedVersion()
{
    return selectedVersion;
}

Version* GraphWidget::getLocalHeadVersion() const
{
    return localHeadVersion;
}

void GraphWidget::resetDiff()
{
    resetMatches();

    compareTree->resetCompareTree();

    fromVersions = QSet<Version*>();
    toVersion = NULL;
    fromToInfo->hide();

    fromHashSave.clear();
    toHashSave = QString();
}

float GraphWidget::getXFactor() const
{
    return 16.0 * xfactor;
}

float GraphWidget::getYFactor() const
{
    return 12.8 * yfactor;
}

bool GraphWidget::getTopDownView() const
{
    return topDownView;
}

void GraphWidget::saveImportantVersions()
{
    fromHashSave.clear();
    foreach(Version * it, fromVersions)
    {
        fromHashSave.push_back(it->getHash());
    }
    toHashSave = toVersion ? toVersion->getHash() : QString();
    fromVersions.clear();
    toVersion = NULL;

    selectedVersionHash = selectedVersion ? selectedVersion->getHash() : QString();
    selectedVersion = NULL;
}

bool GraphWidget::restoreImportantVersions()
{
    fromVersions.clear();
    toVersion = NULL;
    selectedVersion = NULL;

    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* v = dynamic_cast<Version*>(it);

        if (!v || v->getHash().isEmpty())
            continue;

        if (fromHashSave.contains(v->getHash()))
        {
            fromVersions.insert(v);
            v->setMatched(true);
        }
        if (toHashSave.isEmpty() == false && v->getHash() == toHashSave)
        {
            toVersion = v;
            v->setMatched(true);
        }
        if (selectedVersionHash.isEmpty() == false && selectedVersionHash == v->getHash())
        {
            selectedVersion = v;
            selectedVersion->setSelected(true);
        }
    }

    if (selectedVersion == NULL && selectedVersionHash.isEmpty() == false)
    {
        // previous selection, but no new node is matching
        selectedVersion = findVersion(selectedVersionHash);
        if (!selectedVersion)
        {
            selectedVersion = gitlogSingle(selectedVersionHash, true);
            selectedVersion->hide();
            scene()->addItem(selectedVersion);
            if (fromHashSave.contains(selectedVersionHash))
            {
                fromVersions.insert(selectedVersion);
            }
        }
    }

    // everything restored ?
    bool success = (fromVersions.size() == fromHashSave.size())
        && (toHashSave.isEmpty() || toVersion != NULL);

    if (success == false)
    {
        resetDiff();
    }
    else
    {
        if (fromToInfo)
        {
            fromToInfo->setFromToPosition(fromVersions, toVersion);
            fromToInfo->setVisible(success);
        }

        fillCompareWidgetFromToInfo();
    }

    return success;
}
