/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.2-0                */
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
    branchVersion(NULL),
    mwin(_parent),
    compareTree(NULL),
    connectorStyle(0),
    maxLines(0),
    currentLines(0),
    shortHashes(false),
    topDownView(false),
    remotes(false),
    foldHead(false),
    xfactor(1),
    yfactor(1),
    selectedVersion(NULL)
{

    if (mwin)
    {
        maxLines = mwin->getMaxLines();
        connectorStyle = mwin->getConnectorStyle();
        shortHashes = mwin->getShortHashes();
        topDownView = mwin->getTopDownView();
        remotes = mwin->getRemotes();
        foldHead = mwin->getFoldHead();
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

    QStringList nodeNames;

    nodeNames << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H"
              << "I" << "J" << "K" << "L" << "M" << "N" << "O";

    foreach (const QString &n, nodeNames)
    {
        QString line = "#0#0##(tag: " + n + ")#";
        QStringList parts = line.split(QChar('#'));

        nodes[n] = new Version(globalVersionInfo, this);
        nodes[n]->processGitLogInfo(line, parts);

        scene()->addItem(nodes[n]);
    }

    QStringList edgeData;

    edgeData << "OE" << "OF" << "ON" << "EA" << "ED" << "DB" << "DC" << "NG"
             << "NM" << "MH" << "MI" << "MJ" << "MK" << "ML";

    foreach (const QString &e, edgeData)
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
    setMinSize();
}

void GraphWidget::mousePressEvent(QMouseEvent* _event)
{
    if (_event->button() == Qt::MidButton
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
#if QT_VERSION < 0x050000
    scaleView(pow((double) 1.3, event->delta() / 240.0));
#else
    scaleView(pow((double) 1.3, event->angleDelta().y() / 240.0));
#endif
}

void GraphWidget::keyPressEvent(QKeyEvent* _event)
{
    switch (_event->key())
    {
        case Qt::Key_W:
            verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition() - 2);
            break;
        case Qt::Key_F:
            if (mwin && Qt::ControlModifier == QApplication::keyboardModifiers())
            {
                mwin->getSearchDock()->show();
                mwin->getSearchWidget()->setFocus();
            }
            break;
        case Qt::Key_S:
            verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition() + 2);
            break;
        case Qt::Key_D:
            horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->sliderPosition() + 2);
            break;
        case Qt::Key_A:
            horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->sliderPosition() - 2);
            break;
        case Qt::Key_Plus:
            zoomIn();
            break;
        case Qt::Key_Minus:
            zoomOut();
            break;
        case Qt::Key_1:
            QGraphicsView::fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
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

Version* GraphWidget::gitlogSingle(QString _hash)
{
    if (localRepositoryPath.isEmpty())
    {
        return NULL;
    }

    QString cmd = "git -C "
        + localRepositoryPath
        + " log --graph -1 --pretty=\"#"
        + (shortHashes ? "%h" : "%H")
        + "#%at#%an#%d#%s\"";

    if (_hash.isEmpty() == false)
        cmd += " " + _hash;

    QList<QString> cache;

    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    QString line = cache.front();

    QStringList parts = line.split(QChar('#'));

    // abort, if too short...
    if (parts.size() < 5)
    {
        cerr << "Error: Input too short " << line.toUtf8().data() << endl;
        return NULL;
    }
    // check if the Version object already exists
    QString hash = parts.at(1);
    Version* v = new Version(globalVersionInfo, this);

    // if the key information has already been parsed, use it
    v->setKeyInformation(keyInformationCache.value(hash, QMap<QString, QStringList>()));

    // init or update
    if (v->processGitLogInfo(line, parts))
    {
        keyInformationCache[hash] = v->getKeyInformation();
    }

    mwin->getTagWidget()->addData(v->getKeyInformation());

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

void GraphWidget::process(QList<QString> _cache)
{
    // cerr << "process start " << timestamp() << endl;

    // reset local head
    branchVersion = NULL;

    connectorStyle = mwin->getConnectorStyle();

    clear();

    mwin->getTagWidget()->blockSignals(true);
    mwin->getTagWidget()->clear();

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

    foreach (const QString &line, _cache)
    {
        swap_cache.push_front(line);
        linecount++;
        if (linecount > maxLines)
            break;
    }

    currentLines = linecount;

    QString previousTree;
    QRegExp treePattern("^([*\\\\/\\. |\\-_]*[*\\\\/\\.|\\-_]+)");

    foreach (const QString &line, swap_cache)
    {
        int offset = 0;
        int pos = treePattern.indexIn(line, offset);

        if (pos == -1)
            continue;

        QString tree = treePattern.cap(1);

        int len = treePattern.matchedLength();

        offset = pos + len;

        if (previousTree.isEmpty())
        {
            // initial setting
            for (int i = 0; i < tree.size(); i++)
            {
                if (i % 2 != 0)
                    branchslots.push_back(NULL);
                else
                    branchslots.push_back(rootVersion);
            }
            maxTreePatternLength = branchslots.size();
            previousBranchslots = branchslots;
            // +1
            previousBranchslots.push_back(NULL);
        }
        else if (maxTreePatternLength < len)
        {
            branchslots.resize(len + 1);
            maxTreePatternLength = len;
        }

        bool skipUnderscore = false;

        for (int i = 0; i < tree.size(); i++)
        {
            switch (tree[i].toLatin1())
            {
                case '*':
                {
                    Version* parent = branchslots[i];

                    // tokenize
                    QStringList parts = line.mid(offset).split(QChar('#'));

                    // abort, if too short...
                    if (parts.size() < 5)
                    {
                        cerr << "Error: Input too short " << line.toUtf8().data() << endl;
                        break;
                    }
                    // check if the Version object already exists
                    QString hash = parts.at(1);
                    Version* v = new Version(globalVersionInfo, this);

                    // if the key information has already been parsed, use it
                    v->setKeyInformation(keyInformationCache.value(hash, QMap<QString, QStringList>()));

                    // init or update
                    if (v->processGitLogInfo(line.mid(offset), parts))
                    {
                        keyInformationCache[hash] = v->getKeyInformation();
                    }

                    mwin->getTagWidget()->addData(v->getKeyInformation());

                    // main?
                    v->setIsMain(i == 0);

                    scene()->addItem(v);

                    if (parent == NULL)
                        parent = rootVersion;

                    Edge* e = new Edge (parent, v, this, false, parent == rootVersion);

                    scene()->addItem(e);

                    // merge version node ?
                    Version* merge = (tree.size() >= i + 1) ? branchslots[i + 1] : NULL;
                    if (merge && previousTree[i + 1].toLatin1() == '\\' && (previousTree[i].toLatin1() == '|' || previousTree[i].toLatin1() == '*'))
                    {
                        Edge* mergeArrow = new Edge(merge, v, this, true, false);

                        scene()->addItem(mergeArrow);

                        for (int k = i + 1; k < maxTreePatternLength - i; k++)
                        {
                            branchslots[k] = (k % 2 == 0) ? branchslots[k + 1] : NULL;
                        }
                    }
                    // replace parent
                    branchslots[i] = v;
                    // the last line contains the local HEAD version
                    branchVersion = v;
                }
                break;
                case '\\':
                    if ((i % 2) == 1)
                    {
                        branchslots[i] = previousBranchslots[i + 1];
                    }
                    break;
                case '/':
                    if ((i % 2) == 1 && tree[i + 1].toLatin1() != '|')
                    {
                        int k = 2;
                        while (tree[i - k].toLatin1() == '_')
                        {
                            k += 2;
                        }
                        if (k > 2)
                            k += 2;

                        branchslots[i + 1] = previousBranchslots[i + 1 - k];
                    }
                    else if ((i % 2) == 1 && tree[i + 1].toLatin1() == '|' && previousTree[i - 1].toLatin1() == '/')
                    {
                        branchslots[i + 1] = branchslots[i - 1];
                    }
                    break;
                case '_':
                    if (skipUnderscore == false)
                    {
                        skipUnderscore = true;
                        // we have a _ , so check if
                        // previousTree[i-2] is a backslash.
                        // if so, walk right as long as there are
                        // _ present. Transfer branchslots[i-1] to
                        // branchslots[i+x].
                        //
                        // if not, walk right as long as there are
                        // _ present, then take the parent from there
                        // and transfer it into branchslots[i]
                    }
                    // ???
                    break;
                case '|':
                    if (previousTree[i + 1].toLatin1() == '\\' && previousTree[i].toLatin1() == ' ')
                    {
                        branchslots[i] = previousBranchslots[i + 1];
                    }
                    break;
                case '.':
                {
                    // walk left until * is reached.
                    // Add merge arrow to *
                    int k = 2;
                    while (i - k > 0 && tree[i - k].toLatin1() != '*')
                    {
                        k += 2;
                    }

                    if (i - k == 0 && tree[0] != '*')
                    {
                        // error TODO
                    }
                    else
                    {
                        Version* merge = branchslots[i];
                        if (merge)
                        {
                            Edge* e = new Edge(merge, branchslots[i - k], this, true, false);

                            scene()->addItem(e);
                        }
                    }
                }
                break;
                case ' ':
                // do nothing
                default:
                    break;
            }
        }

        previousTree = tree;
        previousBranchslots = branchslots;
    }

    if (branchVersion)
    {
        if (branchVersion->getKeyInformation().contains("HEAD") == false)
        {
            localHeadVersion = gitlogSingle();
            if (localHeadVersion)
            {
                localHeadVersion->hide();
                scene()->addItem(localHeadVersion);
            }
        }
        else
        {
            localHeadVersion = branchVersion;
            localHeadVersion->setIsFoldable(foldHead);
        }
    }

    rootVersion->collectFolderVersions(rootVersion, NULL);
    normalizeGraph();
    setMinSize();

    mwin->getTagWidget()->blockSignals(false);
    mwin->getTagWidget()->setDefault();

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
    foreach(const QString &str, cache)
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
        mwin->getFromComboBox()->addItem(it->getCommitDate(), QVariant::fromValue(VersionPointer(it)));
    }
    mwin->getCompareTreeFromPushButton()->setEnabled(fromVersions.size() > 0);

    // to version part
    mwin->getCompareTreeToTextEdit()->clear();
    if (toVersion)
    {
        toVersion->setMatched(true);
        mwin->getToDateLabel()->setText(toVersion->getCommitDate());
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

    fromVersions = QList<Version*>();
    fromVersions.push_back(localHeadVersion);
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

    fromVersions = QList<Version*>();
    fromVersions.push_back(_v);
    toVersion = NULL;

    fillCompareWidgetFromToInfo();

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

    fromVersions = QList<Version*>();
    fromVersions.push_back(localHeadVersion);
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

    fromVersions = QList<Version*>();
    fromVersions.push_back(_v1);
    toVersion = _v2;

    fillCompareWidgetFromToInfo();

    QStringList fromVersionHashes;

    fromVersionHashes.push_back(_v1->getHash());
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

void GraphWidget::focusVersion(const Version* _v)
{
    focusNeighbourBox(_v->getNeighbourBox());
}

void GraphWidget::focusCurrent()
{
    if (branchVersion)
    {
        resetMatches();
        branchVersion->setMatched(true);
        focusVersion(branchVersion);
    }
    else if (localHeadVersion)
    {
        resetMatches();
        localHeadVersion->setMatched(true);
        focusVersion(localHeadVersion);
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
        focusVersion(v);
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
        focusVersion(toVersion);
    }
}

void GraphWidget::focusNeighbourBox(const QRectF& _rect)
{
    pan = false;
    QGraphicsView::fitInView(_rect, Qt::KeepAspectRatio);
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
    if (_v)
    {
        QPoint restoreVersionPosition = mapFromScene(_v->scenePos());

        normalizeGraph();
        setMinSize(false);

        horizontalScrollBar()->setValue(0);
        verticalScrollBar()->setValue(0);

        QPoint shift = mapFromScene(_v->scenePos()) - restoreVersionPosition;
        horizontalScrollBar()->setValue(shift.x());
        verticalScrollBar()->setValue(shift.y());
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

bool GraphWidget::focusElement(const QString& _text, bool _exactMatch)
{
    pan = false;
    resetMatches();
    if (_text.isEmpty())
    {
        QGraphicsView::fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        return false;
    }

    QList<QGraphicsItem*> hits;
    QRegExp pattern(_text);

    foreach(QGraphicsItem * it, scene()->items())
    {
        if (it->type() != QGraphicsItem::UserType + 1)
            continue;

        Version* n = dynamic_cast<Version*>(it);

        if (n && n->findMatch(pattern, _text, _exactMatch))
        {
            hits.push_back(it);
        }
    }

    if (hits.size() > 0)
    {
        // ensure visibility
        updateGraphFolding();

        QGraphicsItem* it = hits.front();
        QRectF tmp = QRectF(-10, -10, 20, 20).translated(it->scenePos());

        foreach(QGraphicsItem * it, hits)
        {
            tmp |= QRectF(-10, -10, 20, 20).translated(it->scenePos());
        }

        tmp.adjust(-getXFactor(), -getYFactor(), getXFactor(), getYFactor());
        QGraphicsView::fitInView(tmp, Qt::KeepAspectRatio);
    }
    else
    {
        QGraphicsView::fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        return false;
    }
    return true;
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

    if (remotes != mwin->getRemotes())
    {
        remotes = mwin->getRemotes();
        updateAll = true;
    }

    if (foldHead != mwin->getFoldHead())
    {
        foldHead = mwin->getFoldHead();
        updateAll = true;
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
    textColor = mwin->getPreferencesColor("colorText");
    foldedColor = mwin->getPreferencesColor("colorFolded");
    unfoldedColor = mwin->getPreferencesColor("colorUnfolded");
    fileConstraintColor = mwin->getPreferencesColor("colorFileConstraint");
    forceUpdate();
}

void GraphWidget::foldAll()
{
    rootVersion->foldRecurse(true);
    updateGraphFolding();
}

void GraphWidget::unfoldAll()
{
    rootVersion->foldRecurse(false);
    updateGraphFolding();
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
        }

        if (uit->type() == QGraphicsItem::UserType + 2)
        {
            it = uit;
            break;
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
            /*
             * TODO check if it does make sense to hide/show subtrees
               if (v->getSubtreeHidden())
               {
                action = menu.addAction(QString("Show subtree"));
                connect(action, SIGNAL(triggered()), v, SLOT(showSubtree()));
               }
               else
               {
                action = menu.addAction(QString("Hide subtree"));
                connect(action, SIGNAL(triggered()), v, SLOT(hideSubtree()));
               }
               menu.addSeparator();
             */
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

const QList<Version*>& GraphWidget::getPredecessors() const
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

    fromVersions = QList<Version*>();
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
            fromVersions.push_back(v);
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
            selectedVersion = gitlogSingle(selectedVersionHash);
            selectedVersion->hide();
            scene()->addItem(selectedVersion);
            if (fromHashSave.contains(selectedVersionHash))
            {
                fromVersions.push_back(selectedVersion);
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
