// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gccreator.h"

#include "app.h"
#include "erroritem.h"
#include "gcfile.h"
#include "point.h"
#include "project.h"
#include "voroni/jc_voronoi.h"
//#include "errno.h"
//#include "ft_model.h"
//#include "forms/bridgeitem.h"
//#include "gccreator.h"
//#include "gcvoronoi.h"
//#include "locale.h"
//#include "scene.h"
//#include "settings.h"
//#include "stdio.h"
//#include "string.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <QStack>
#include <algorithm>
#include <limits>
#include <stdexcept>

#include <future>
#include <thread>

#include "leakdetector.h"

void dbgPaths(Paths ps, const QString& fileName, bool close, const Tool& tool)
{
    if (ps.empty()) {

        return;
    }
    for (size_t i = 0; i < ps.size(); ++i)
        if (ps[i].empty())
            ps.erase(ps.begin() + i--);

    if (close)
        for (Path& p : ps)
            p.push_back(p.front());

    //assert(ps.isEmpty());
    GCode::GCodeParams gcp { tool, 0.0, GCode::Profile };
    auto file = new GCode::File({ ps }, gcp);
    file->setFileName(fileName + "_" + tool.name());
    //file->itemGroup()->setPen({ Qt::green, 0.0 });
    emit App::project()->addFileDbg(file);

    //    static QMutex m;
    //    m.lock();

    //    if (ps.isEmpty()) {

    //        return;
    //    }
    //    const auto polygons = toQPolygons(ps);
    //    for (auto& path : polygons) {
    //        if (path.size() < 2)
    //            continue;
    //        auto pl = new Shapes::PolyLine(path[0], path[1]);
    //        for (size_t i = 2; i < path.size(); ++i)
    //            pl->addPt(path[i]);
    //        //        for (auto& p : path.mid(3))
    //        //            pl->addPt(p);
    //        pl->setSelected(true);
    //        pl->setSelected(false);
    //        App::project()->addShape(pl);
    //    }
    //    m.unlock();
};

namespace GCode {

Creator::Creator() { }

void Creator::reset()
{
    ProgressCancel::reset();
    //    setCreator(this);

    m_file = nullptr;

    m_workingPs.clear();
    m_workingRawPs.clear();
    m_returnPs.clear();
    m_returnPss.clear();
    m_supportPss.clear();
    m_groupedPss.clear();

    m_toolDiameter = 0.0;
    m_dOffset = 0.0;
    m_stepOver = 0.0;
}

Creator::~Creator() { ProgressCancel::reset(); }

Pathss& Creator::groupedPaths(Grouping group, cInt k)
{
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddPaths(m_workingPs, ptSubject, true);
    IntRect r(clipper.GetBounds());
    Path outer = {
        Point64(r.left - k, r.top - k),
        Point64(r.right + k, r.top - k),
        Point64(r.right + k, r.bottom + k),
        Point64(r.left - k, r.bottom + k),
    };
    // ReversePath(outer);
    clipper.AddPath(outer, ptSubject, true);
    clipper.Execute(ctUnion, polyTree, pftNonZero);
    /****************************/
    std::function<void(PolyNode*, Grouping)> grouping = [&grouping, this](PolyNode* node, Grouping group) {
        Paths paths;
        switch (group) {
        case CutoffPaths:
            if (!node->IsHole()) {
                Path& path = node->Contour;
                paths.push_back(path);
                for (size_t i = 0, end = node->ChildCount(); i < end; ++i) {
                    path = node->Childs[i]->Contour;
                    paths.push_back(path);
                }
                m_groupedPss.push_back(paths);
            }
            for (size_t i = 0, end = node->ChildCount(); i < end; ++i)
                grouping(node->Childs[i], group);
            break;
        case CopperPaths:
            if (node->IsHole()) {
                Path& path = node->Contour;
                paths.push_back(path);
                for (size_t i = 0, end = node->ChildCount(); i < end; ++i) {
                    path = node->Childs[i]->Contour;
                    paths.push_back(path);
                }
                m_groupedPss.push_back(paths);
            }
            for (size_t i = 0, end = node->ChildCount(); i < end; ++i)
                grouping(node->Childs[i], group);
            break;
        }
    };
    /*********************************/
    m_groupedPss.clear();
    grouping(polyTree.GetFirst(), group);

    if (group == CutoffPaths) {
        if (m_groupedPss.size() > 1 && m_groupedPss.front().size() == 2)
            m_groupedPss.erase(m_groupedPss.begin());
    }

    return m_groupedPss;
}
////////////////////////////////////////////////////////////////
/// \brief Creator::addRawPaths
/// \param paths
///
void Creator::addRawPaths(Paths rawPaths)
{
    if (rawPaths.empty())
        return;

    //    if (m_gcp.side() == On) {
    //        m_workingRawPs.push_back(rawPaths);
    //        return;
    //    }

    for (size_t i = 0; i < rawPaths.size(); ++i)
        if (rawPaths[i].size() < 2)
            rawPaths.erase(rawPaths.begin() + i--);

    const double glueLen = App::project()->glue() * uScale;
    Clipper clipper;
    for (size_t i = 0; i < rawPaths.size(); ++i) {
        Point64& pf = rawPaths[i].front();
        Point64& pl = rawPaths[i].back();
        if (rawPaths[i].size() > 3 && (pf == pl || pf.distTo(pl) < glueLen)) {
            clipper.AddPath(rawPaths[i], ptSubject, true);
            rawPaths.erase(rawPaths.begin() + i--);
        }
    }

    mergeSegments(rawPaths, App::project()->glue() * uScale);

    for (Path& path : rawPaths) {
        Point64& pf = path.front();
        Point64& pl = path.back();
        if (path.size() > 3 && (pf == pl || pf.distTo(pl) < glueLen))
            clipper.AddPath(path, ptSubject, true);
        else
            m_workingRawPs.push_back(path);
    }

    IntRect r(clipper.GetBounds());
    int k = uScale;
    Paths paths;
    clipper.AddPath({ { r.left - k, r.bottom + k },
                        { r.right + k, r.bottom + k },
                        { r.right + k, r.top - k },
                        { r.left - k, r.top - k } },
        ptClip, true);
    clipper.Execute(ctXor, paths, pftEvenOdd);
    m_workingPs.insert(m_workingPs.end(), paths.begin() + 1, paths.end()); // paths.takeFirst();
}

void Creator::addSupportPaths(Pathss supportPaths) { m_supportPss.append(supportPaths); }

void Creator::addPaths(const Paths& paths) { m_workingPs.append(paths); }

void Creator::createGc()
{
    QElapsedTimer t;
    t.start();
    try {
        if (type() == Profile || //
            type() == Pocket || //
            type() == Raster) {
            switch (m_gcp.side()) {
            case Outer:
                groupedPaths(CutoffPaths, m_gcp.tools.front().getDiameter(m_gcp.getDepth()) * uScale + 100);
                createability(CutoffPaths);
                break;
            case Inner:
                groupedPaths(CopperPaths);
                createability(CopperPaths);
                break;
            case On:
                break;
            }
        }
        if (!getCancel())
            create();
        qWarning() << "Creator::createGc() finish:" << t.elapsed();
    } catch (const cancelException& e) {
        qWarning() << "Creator::createGc() canceled:" << e.what() << t.elapsed();
    } catch (const std::exception& e) {
        qWarning() << "Creator::createGc() exeption:" << e.what() << t.elapsed();
    } catch (...) {
        qWarning() << "Creator::createGc() exeption:" << errno << t.elapsed();
    }
}

void Creator::createGc(const GCodeParams& gcp)
{
    m_gcp = gcp;
    createGc();
}

void Creator::cancel() // direct connection!!
{
    qDebug(__FUNCTION__);
    setCancel(true);
    condition.wakeAll();
}

void Creator::proceed() // direct connection!!
{
    qDebug(__FUNCTION__);
    setCancel(false);
    condition.wakeAll();
}

GCode::File* Creator::file() const { return m_file; }

std::pair<int, int> Creator::getProgress()
{
    return { static_cast<int>(getMax()), static_cast<int>(getCurrent()) };
}

void Creator::stacking(Paths& paths)
{
    if (paths.empty())
        return;
    QElapsedTimer t;
    t.start();
    PolyTree polyTree;
    {
        Clipper clipper;
        clipper.AddPaths(paths, ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = uScale;
        Path outer = { Point64(r.left - k, r.bottom + k), Point64(r.right + k, r.bottom + k),
            Point64(r.right + k, r.top - k), Point64(r.left - k, r.top - k) };
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftEvenOdd);
        paths.clear();
    }
    sortPolyNodeByNesting(polyTree);
    m_returnPss.clear();
    /***********************************************************************************************/
    t.start();

    auto mathBE = [this](Paths& paths, Path& path, std::pair<size_t, size_t> idx) -> bool {
        QList<std::iterator_traits<Path::iterator>::difference_type> list;
        list.push_back(idx.first);
        for (size_t i = paths.size() - 1, index = idx.first; i; --i) {
            double d = std::numeric_limits<double>::max();
            Point64 pt;
            for (const Point64& pts : paths[i - 1]) {
                double l = pts.distTo(paths[i][index]);
                if (d >= l) {
                    d = l;
                    pt = pts;
                }
            }
            if (d <= m_toolDiameter) {
                list.prepend(paths[i - 1].indexOf(pt));
                index = list.front();
            } else
                return false;
        }
        for (size_t i = 0; i < paths.size(); ++i)
            std::rotate(paths[i].begin(), paths[i].begin() + list[i], paths[i].end());
        std::rotate(path.begin(), path.begin() + idx.second, path.end());
        return true;
    };

    using Worck = std::pair<PolyNode*, bool>;
    std::function<void(Worck)> stacker = [&stacker, &mathBE, this](Worck w) {
        auto [node, newPaths] = w;
        if (!m_returnPss.empty() || newPaths) {
            Path path(node->Contour);
            if (!(m_gcp.convent() ^ !node->IsHole()) ^ !(m_gcp.side() == Outer))
                ReversePath(path);
            //            if (App::settings().gbrCleanPolygons())
            //                CleanPolygon(path, uScale * 0.0005);
            if (m_returnPss.empty() || newPaths) {
                m_returnPss.push_back({ path });
            } else {
                // check distance;
                std::pair<size_t, size_t> idx;
                double d = std::numeric_limits<double>::max();
                for (size_t id = 0; id < m_returnPss.back().back().size(); ++id) {
                    const Point64& ptd = m_returnPss.back().back()[id];
                    for (size_t is = 0; is < path.size(); ++is) {
                        const Point64& pts = path[is];
                        const double l = ptd.distTo(pts);
                        if (d >= l) {
                            d = l;
                            idx.first = id;
                            idx.second = is;
                        }
                    }
                }
                if (d <= m_toolDiameter && mathBE(m_returnPss.back(), path, idx))
                    m_returnPss.back().push_back(path);
                else
                    m_returnPss.push_back({ path });
            }
            for (size_t i = 0, end = node->ChildCount(); i < end; ++i)
                stacker({ node->Childs[i], static_cast<bool>(i) });
        } else { // Start from here
            for (size_t i = 0, end = node->ChildCount(); i < end; ++i)
                stacker({ node->Childs[i], true });
        }
        //PROG setProgInc();
    };
    /***********************************************************************************************/
    //PROG .3setProgMax(polyTree.Total());
    stacker({ polyTree.GetFirst(), false });

    for (Paths& retPaths : m_returnPss) {
        std::reverse(retPaths.begin(), retPaths.end());
        for (size_t i = 0; i < retPaths.size(); ++i) {
            if (retPaths[i].empty())
                retPaths.erase(retPaths.begin() + i--);
        }
        for (Path& path : retPaths)
            path.push_back(path.front());
    }
    sortB(m_returnPss);
    //    for (auto& paths : m_returnPss) {
    //        bool ff, fl;
    //        for (size_t f = 0, l = paths.size() - 1; f < l; ++f, --l) {
    //            if (f) {
    //                if (!(m_gcp.convent() ^ (ff ? Area(paths[f]) > 0 : Area(paths[f]) < 0)) ^ (m_gcp.side() == Outer))
    //                    ReversePath(paths[f]);
    //                if (!(m_gcp.convent() ^ (fl ? Area(paths[l]) > 0 : Area(paths[l]) < 0)) ^ (m_gcp.side() == Outer))
    //                    ReversePath(paths[l]);
    //            } else {
    //                ff = Area(paths[f]) > 0;
    //                fl = Area(paths[l]) > 0;
    //            }
    //        }
    //    }
}

void Creator::mergeSegments(Paths& paths, double glue)
{
    size_t size;
    do {
        size = paths.size();
        for (size_t i = 0; i < paths.size(); ++i) {
            for (size_t j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                if (i >= paths.size() || j >= paths.size()) {
                    i = -1;
                    j = 0;
                    break;
                }
                Point64& pif = paths[i].front();
                Point64& pil = paths[i].back();
                Point64& pjf = paths[j].front();
                Point64& pjl = paths[j].back();
                if (pil == pjf) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                if (pif == pjl) {
                    paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[i].end());
                    paths.erase(paths.begin() + i--);
                    break;
                }
                if (pil == pjl) {
                    ReversePath(paths[j]);
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
            }
        }
    } while (size != paths.size());
    if (qFuzzyIsNull(glue))
        return;
    do {
        size = paths.size();
        for (size_t i = 0; i < paths.size(); ++i) {
            for (size_t j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                Point64& pif = paths[i].front();
                Point64& pil = paths[i].back();
                Point64& pjf = paths[j].front();
                Point64& pjl = paths[j].back();
                if (pil.distTo(pjf) < glue) {
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
                if (pif.distTo(pjl) < glue) {
                    paths[j].insert(paths[j].end(), paths[i].begin() + 1, paths[i].end());
                    paths.erase(paths.begin() + i--);
                    break;
                }
                if (pil.distTo(pjl) < glue) {
                    ReversePath(paths[j]);
                    paths[i].insert(paths[i].end(), paths[j].begin() + 1, paths[j].end());
                    paths.erase(paths.begin() + j--);
                    continue;
                }
            }
        }
    } while (size != paths.size());
}

void Creator::sortPolyNodeByNesting(PolyNode& polynode, bool beSort)
{
    int nestCtr = 0;
    if (!beSort) {
        std::function<int(PolyNode&)> sorter = [&sorter, &nestCtr](PolyNode& polynode) {
            ++nestCtr;
            if (polynode.ChildCount() == 0) {
                return nestCtr--;
            } else if (polynode.ChildCount() == 1) {
                return std::max(nestCtr--, sorter(*polynode.Childs.front()));
            } else {
                std::map<int, std::vector<PolyNode*>, std::greater<>> map;
                for (auto node : polynode.Childs)
                    map[sorter(*node)].emplace_back(node);
                size_t i = polynode.ChildCount();
                for (auto& [nest, nodes] : map) {
                    for (auto node : nodes)
                        polynode.Childs[--i] = node;
                }
                return std::max(nestCtr--, map.begin()->first);
            }
        };
        sorter(polynode);
    } else {

        auto searchMinimumDistances = [/*this*/](Paths& paths) {
            std::vector<jcv_point> points;

            size_t id = 0;
            for (const Path& path : paths)
                id += path.size();
            points.reserve(id);
            id = 0;

            for (const Path& path : paths) {
                for (const auto& point : path)
                    points.push_back({
                        //
                        static_cast<jcv_real>(point.X),
                        static_cast<jcv_real>(point.Y),
                        static_cast<int>(id) //
                    });
                ++id;
            }

            Clipper clipper;
            clipper.AddPaths(paths, ptClip, true);
            const IntRect r(clipper.GetBounds());

            using Key = std::pair<int, int>;
            using Md = std::map<Key, double>;
            using Mdr = std::map<double, std::vector<Key>>;
            Md minimumDistances;

            //            Paths dbg;
            {
                const cInt fo = uScale;
                jcv_rect bounding_box = {
                    { static_cast<jcv_real>(r.left - fo), static_cast<jcv_real>(r.top - fo) },
                    { static_cast<jcv_real>(r.right + fo), static_cast<jcv_real>(r.bottom + fo) }
                };
                jcv_diagram diagram;
                jcv_diagram_generate(points.size(), points.data(), &bounding_box, nullptr, &diagram);
                const jcv_edge* edge = jcv_diagram_get_edges(&diagram);
                while (edge) {
                    if (edge->sites[0] && edge->sites[1]) {
                        auto p1 = edge->sites[0]->p;
                        auto p2 = edge->sites[1]->p;
                        if (p1.id != p2.id) {
                            //                            dbg.push_back({ IntPoint(edge->pos[0].x, edge->pos[0].y), IntPoint(edge->pos[1].x, edge->pos[1].y) });
                            Key key { std::min(p1.id, p2.id), std::max(p1.id, p2.id) };
                            double distance = QLineF(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y)).length();
                            if (auto& md = minimumDistances[key]; md == 0.0)
                                md = distance;
                            else if (md > distance)
                                md = distance;
                        }
                    }
                    edge = jcv_diagram_get_next_edge(edge);
                }
                jcv_diagram_free(&diagram);
            }

            //            mergeSegments(dbg, uScale * 0.1);
            //            dbgPaths(dbg, "searchMinimumDistances");

            Mdr minimumDistancesRev;
            for (auto [key, val] : minimumDistances)
                minimumDistancesRev[val].emplace_back(key);

            std::vector<size_t> idx;
            std::set<int> set;
            idx.reserve(paths.size() - 1);
            while (idx.size() < idx.capacity()) {
                qDebug() << "idx" << idx.size();
                for (auto& [dist, idxses] : minimumDistancesRev) {
                    if (!idxses.size()) {
                        minimumDistancesRev.extract(dist);
                        break;
                    }
                    int ctr = 0;
                    for (auto [_first, _second] : idxses) {
                        if (!idx.size() && !_first) {
                            if (set.insert(_second).second) {
                                idx.push_back(_second);
                                idxses.erase(idxses.begin() + ctr);
                                ctr = 0;
                                break;
                            }
                        } else if (idx.size() && std::min(_first, _second)) {
                            if (_first == static_cast<int>(idx.back()) && set.insert(_second).second) {
                                idx.push_back(_second);
                                idxses.erase(idxses.begin() + ctr);
                                ctr = 0;
                                break;
                            }
                            if (_second == static_cast<int>(idx.back()) && set.insert(_first).second) {
                                idx.push_back(_first);
                                idxses.erase(idxses.begin() + ctr);
                                ctr = 0;
                                break;
                            }
                        }
                        ++ctr;
                    }
                    if (!ctr)
                        break;
                }
            }
            return idx;
        };

        bool stage = false;
        std::function<int(PolyNode&)> sorter = [&sorter, &searchMinimumDistances, &nestCtr, &stage](PolyNode& polynode) {
            ++nestCtr;
            qDebug() << "nestCtr" << nestCtr;
            if (polynode.ChildCount() == 0) {
                return nestCtr--;
            } else if (polynode.ChildCount() == 1) {
                return std::max(nestCtr--, sorter(*polynode.Childs.front()));
            } else {
                std::map<int, std::vector<PolyNode*>, std::greater<>> nMap;

                for (auto node : polynode.Childs)
                    nMap[sorter(*node)].emplace_back(node);

                size_t i = polynode.ChildCount();

                if (stage && i > 1 && nMap.size() == 1 && polynode.Parent) {
                    for (auto& [nest, nodes] : nMap) {
                        Paths paths;
                        paths.reserve(nodes.size() + 1);
                        paths.emplace_back(polynode.Parent->Childs.back()->Contour);
                        for (auto& node : nodes)
                            paths.emplace_back(node->Contour);
                        for (auto idx : searchMinimumDistances(paths))
                            polynode.Childs[--i] = nodes[idx - 1];
                    }
                } else {
                    for (auto& [nest, nodes] : nMap) {
                        if (nodes.size() > 1) {
                            Paths paths;
                            paths.reserve(nodes.size() + 1);
                            if (i == polynode.ChildCount()) {
                                paths.emplace_back(polynode.Contour);
                                for (auto& node : nodes)
                                    paths.emplace_back(node->Contour);
                            } else {
                                paths.emplace_back(polynode.Childs[i]->Contour);
                                for (auto& node : nodes)
                                    paths.emplace_back(node->Contour);
                            }
                            for (auto idx : searchMinimumDistances(paths))
                                polynode.Childs[--i] = nodes[idx - 1];
                        } else
                            for (auto node : nodes)
                                polynode.Childs[--i] = node;
                    }
                }
                return std::max(nestCtr--, nMap.begin()->first);
            }
        };
        sorter(polynode);
        stage = true;
        sorter(polynode);
    }
}

void Creator::isContinueCalc()
{
    emit errorOccurred();
    mutex.lock();
    condition.wait(&mutex);
    mutex.unlock();
    items.clear();
    //    if (m_cancel)
    //        throw cancelException("canceled by user");
}

bool Creator::createability(bool side)
{
    QElapsedTimer t;
    t.start();
    //    Paths wpe;
    const double d = m_gcp.tools.back().getDiameter(m_gcp.getDepth()) * uScale;
    const double r = d * 0.5;
    const double testArea = d * d - M_PI * r * r;

    Paths srcPaths;
    for (size_t pIdx = 0; pIdx < m_groupedPss.size(); ++pIdx) {
        srcPaths.append(m_groupedPss[pIdx]);
    }
    qDebug() << __FUNCTION__ << "insert" << t.elapsed();

    Paths frPaths;
    {
        ClipperOffset offset(uScale);
        offset.AddPaths(srcPaths, jtRound, etClosedPolygon);
        offset.Execute(frPaths, -r);
        //        if (App::settings().gbrCleanPolygons())
        //            CleanPolygons(frPaths, uScale * 0.0005);
        offset.Clear();
        offset.AddPaths(frPaths, jtRound, etClosedPolygon);
        offset.Execute(frPaths, r + 100);
    }
    qDebug() << __FUNCTION__ << "offset" << t.elapsed();
    if (side == CopperPaths)
        ReversePaths(srcPaths);
    {
        Clipper clipper;
        clipper.AddPaths(frPaths, ptClip);
        clipper.AddPaths(srcPaths, ptSubject);
        clipper.Execute(ctDifference, frPaths, pftPositive);
    }
    qDebug() << __FUNCTION__ << "clipper1" << t.elapsed();
    QString last(msg);
    if (!frPaths.empty()) {
        PolyTree polyTree;
        {
            Clipper clipper;
            clipper.AddPaths(frPaths, ptSubject, true);
            IntRect rect(clipper.GetBounds());
            int k = uScale;
            Path outer = { Point64(rect.left - k, rect.bottom + k), Point64(rect.right + k, rect.bottom + k),
                Point64(rect.right + k, rect.top - k), Point64(rect.left - k, rect.top - k) };
            clipper.AddPath(outer, ptSubject, true);
            clipper.Execute(ctUnion, polyTree, pftEvenOdd);
        }
        qDebug() << __FUNCTION__ << "clipper2" << t.elapsed();

        auto test = [&srcPaths, side](PolyNode* node) -> bool {
            if (node->ChildCount() > 0) {
                return true;
            } else {
                QSet<size_t> skip;
                for (auto& point : node->Contour) {
                    for (size_t i = 0; i < srcPaths.size(); ++i) {
                        if (skip.contains(i))
                            continue;
                        const auto& path = srcPaths[i];
                        if (int fl = PointInPolygon(point, path); side == CopperPaths || fl == -1) { ////////////////
                            skip.insert(i);
                            break;
                        }
                    }
                }
                if (skip.size() > 1) {
                    return true;
                }
            }
            return false;
        };

        setMax(frPaths.size());
        setCurrent();

        msg = tr("Creativity check");
        const std::function<void(PolyNode*, double)> creator = [&creator, test, testArea, this](PolyNode* node, double area) {
            if (node && !node->IsHole()) { // init run
                for (size_t i = 0; i < node->ChildCount(); ++i) {
                    if (area = -Area(node->Childs[i]->Contour); testArea < area) {
                        if (test(node->Childs[i])) {
                            creator(node->Childs[i], area);
                        }
                    }
                }
            } else {
                static Paths paths;
                paths.clear();
                paths.reserve(node->ChildCount() + 1);
                paths.push_back(std::move(node->Contour)); // init path
                for (size_t i = 0; i < node->ChildCount(); ++i) {
                    area += Area(node->Childs[i]->Contour);
                    paths.push_back(std::move(node->Childs[i]->Contour));
                }
                incCurrent();
                items.push_back(new ErrorItem(paths, area * dScale * dScale));
                for (size_t i = 0; i < node->ChildCount(); ++i) {
                    creator(node->Childs[i], area);
                }
            }
        };
        creator(polyTree.GetFirst(), 0);
    }
    qDebug() << __FUNCTION__ << "creator" << t.elapsed();
    msg = last;
    if (!items.empty())
        isContinueCalc();
    return true;
}

GCodeParams Creator::getGcp() const { return m_gcp; }

void Creator::setGcp(const GCodeParams& gcp)
{
    m_gcp = gcp;
    reset();
}

//void Creator:://PROG .3setProgMax(int progressMax)
//{
//    //        if (App::set_creator != nullptr)
//    //PROG  m_progressMax += progressMax;
//}

//void Creator:: //PROG //PROG .3setProgMaxAndVal(int progressMax, int progressVal)
//{
//    //        if (m_cancel) {
//    //            m_cancel = false;
//    //            throw cancelException("canceled by user");
//    //        }
//    //PROG m_progressVal = progressVal;
//    //PROG  m_progressMax = progressMax;
//}

//void Creator:://PROG setProgInc()
//{
//    //        if (m_cancel) {
//    //            m_cancel = false;
//    //            throw cancelException("canceled by user");
//    //        }
//    if (App::set_creator != nullptr)
//        if (//PROG  m_progressMax < ++//PROG m_progressVal) {
//            if (//PROG  m_progressMax == 0)
//                //PROG  m_progressMax = 100;
//            else
//                //PROG  m_progressMax *= 2;
//        }
//}

Paths& Creator::sortB(Paths& src)
{
    Point64 startPt((Marker::get(Marker::Home)->pos() + Marker::get(Marker::Zero)->pos()));
    for (size_t firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for (size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = startPt.distTo(src[secondIdx].front());
            if (destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].back();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Paths& Creator::sortBE(Paths& src)
{
    Point64 startPt((Marker::get(Marker::Home)->pos() + Marker::get(Marker::Zero)->pos()));
    for (size_t firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        //PROG //PROG .3setProgMaxAndVal(src.size(), firstIdx);
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse = false;
        for (size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = startPt.distTo(src[secondIdx].front());
            const double lenLast = startPt.distTo(src[secondIdx].back());
            if (lenFirst < lenLast) {
                if (destLen > lenFirst) {
                    destLen = lenFirst;
                    swapIdx = secondIdx;
                    reverse = false;
                }
            } else {
                if (destLen > lenLast) {
                    destLen = lenLast;
                    swapIdx = secondIdx;
                    reverse = true;
                }
            }
            if (qFuzzyIsNull(destLen))
                break;
        }
        if (reverse)
            ReversePath(src[swapIdx]);
        startPt = src[swapIdx].back();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Pathss& Creator::sortB(Pathss& src)
{
    Point64 startPt(
        (Marker::get(Marker::Home)->pos() + Marker::get(Marker::Zero)->pos()));

    for (size_t i = 0; i < src.size(); ++i) {
        if (src[i].empty())
            src.erase(src.begin() + i--);
    }
    for (size_t firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for (size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = startPt.distTo(src[secondIdx].front().front());
            if (destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].back().back();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Pathss& Creator::sortBE(Pathss& src)
{
    Point64 startPt(
        (Marker::get(Marker::Home)->pos() + Marker::get(Marker::Zero)->pos()));
    for (size_t firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        size_t swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse = false;
        for (size_t secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = startPt.distTo(src[secondIdx].front().front());
            const double lenLast = startPt.distTo(src[secondIdx].back().back());
            if (lenFirst < lenLast) {
                if (destLen > lenFirst) {
                    destLen = lenFirst;
                    swapIdx = secondIdx;
                    reverse = false;
                }
            } else {
                if (destLen > lenLast) {
                    destLen = lenLast;
                    swapIdx = secondIdx;
                    reverse = true;
                }
            }
        }
        //        if (reverse)
        //            std::reverse(src[swapIdx].begin(), src[swapIdx].end());
        //        startPt = src[swapIdx].back().back();
        if (swapIdx != firstIdx && !reverse) {
            startPt = src[swapIdx].back().back();
            std::swap(src[firstIdx], src[swapIdx]);
        }
    }
    return src;
}

bool Creator::pointOnPolygon(const QLineF& l2, const Path& path, Point64* ret)
{
    const size_t cnt = path.size();
    if (cnt < 2)
        return false;
    QPointF p;
    for (size_t i = 0; i < cnt; ++i) {
        const Point64& pt1 = path[(i + 1) % cnt];
        const Point64& pt2 = path[i];
        QLineF l1(pt1, pt2);

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
        if (QLineF::BoundedIntersection == l1.intersect(l2, &p)) {
#else
        if (QLineF::BoundedIntersection == l1.intersects(l2, &p)) {
#endif
            if (ret)
                *ret = (p);
            return true;
        }
    }

    return false;
}
} // namespace GCode
