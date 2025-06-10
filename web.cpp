#define UIT_VENDORIZE_EMP
#define UIT_SUPPRESS_MACRO_INSEEP_WARNINGS

#include <deque>
#include "emp/math/Random.hpp"
#include "emp/math/math.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/web.hpp"
#include "World.h"
#include "Org.h"
#include "Cell.h"
#include "ConfigSetup.h"
#include "emp/config/ArgManager.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/web/UrlParams.hpp"

emp::web::Document doc("target");
emp::web::Document settings("settings");
emp::web::Document stats("stats");
emp::web::Document tasksDoc("tasks");
emp::web::Document cellsDoc("cells");
// Your configuration object needs to exist outside of the animator class
MyConfigType worldConfig;

struct Stats
{
    int count;
    double min;
    double max;
    double mean;
    double variance;
};

class AEAnimator : public emp::web::Animate
{

    // arena width and height
    const int num_h_boxes = worldConfig.WORLD_LEN();
    const int num_w_boxes = worldConfig.WORLD_WIDTH();
    const double RECT_SIDE = worldConfig.CELL_SIZE();
    const double width{num_w_boxes * RECT_SIDE};
    const double height{num_h_boxes * RECT_SIDE};
    static constexpr double dir_dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    static constexpr double dir_dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    unsigned int max_known_id;
    unsigned int min_known_id;
    long long cycle = 0;

    emp::Random random{worldConfig.SEED()};
    OrgWorld world{random};

    emp::web::Canvas canvas{width, height, "canvas"};

public:
    // Constructor
    AEAnimator()
    {
        SetupCanvas();
        SetupConfigPanel();
        SetupWorld();
    }

    /**
     * Input: None
     *
     * Output: None
     *
     * Purpose: Setup the canvas and add it to the document.
     */
    void SetupCanvas()
    {
        doc << canvas;
        doc << GetToggleButton("Toggle");
        doc << GetStepButton("Step");
    }

    /**
     * Input: None
     *
     * Output: None
     *
     * Purpose: Setup the config panel and add it to the document.
     */
    void SetupConfigPanel()
    {
        emp::prefab::ConfigPanel config_panel(worldConfig);
        for (auto &name : {
                 "FILE_NAME",
                 "UPDATE_NUM",
                 "UPDATE_RECORD_FREQUENCY",
                 "WORLD_LEN",
                 "WORLD_WIDTH",
                 "CELL_SIZE",
             })
        {
            config_panel.ExcludeSetting(name);
        }
        config_panel.SetRange("SEED", "1", "1000");
        config_panel.SetRange("START_NUM", "1", "10");
        config_panel.SetRange("MUTATION_RATE", "0.0", "1.0");
        config_panel.SetRange("MIN_BRIGHT", "0.0", "1.0");
        config_panel.SetRange("MAX_BRIGHT", "0.0", "1.0");
        settings << config_panel;
        auto specs = emp::ArgManager::make_builtin_specs(&worldConfig);
        emp::ArgManager am(emp::web::GetUrlParams(), specs);
        am.UseCallbacks();
        if (am.HasUnused())
            std::exit(EXIT_FAILURE);
    }

    /**
     * Input: None
     *
     * Output: None
     *
     * Purpose: Add the initial organisms to the document.
     */
    void SetupWorld()
    {
        for (int i = 0; i < worldConfig.START_NUM(); i++)
        {
            Organism *new_org = new Organism(&world);
            world.Inject(*new_org);
        }
    }

    static constexpr size_t MAX_HISTORY = 200;
    std::deque<std::vector<size_t>> send_history;
    std::deque<std::vector<size_t>> recv_history;

    /**
     * Input: None
     *
     * Output: Number of organisms in the world, the min and max points among them, plus their mean and variance of points
     *
     * Purpose: Compute the stats to assist in displaying the organisms.
     */
    Stats ComputeStats()
    {
        Stats s;
        s.max = 0.0;
        s.min = std::numeric_limits<double>::infinity();
        double sum = 0.0, sum_sq = 0.0;
        s.count = 0;

        for (int i = 0; i < world.GetSize(); ++i)
        {
            if (!world.IsOccupied(i))
                continue;
            double p = world.GetOrg(i).GetPoints();
            s.max = std::max(s.max, p);
            s.min = std::min(s.min, p);
            sum += p;
            sum_sq += p * p;
            ++s.count;
        }
        if (s.count == 0)
        {
            s.min = 0.0;
            s.max = 0.0;
        }
        s.mean = s.count ? (sum / s.count) : 0.0;
        s.variance = s.count
                         ? (sum_sq / s.count) - (s.mean * s.mean)
                         : 0.0;

        return s;
    }

    /**
     * Input: Collection of Stats
     *
     * Output: None
     *
     * Purpose: Display the stats numerically, along with more stats from the DataMonitor
     */
    void StatsRecord(Stats st)
    {
        stats.Clear();
        stats << "<h4>Point Stats</h4>"
              << "<div>Count:    " << st.count << "</div>"
              << "<div>Min:      " << st.min << "</div>"
              << "<div>Max:      " << st.max << "</div>"
              << "<div>Mean:     " << st.mean << "</div>"
              << "<div>Variance: " << st.variance << "</div>";
    }

    /**
     * Input: None
     *
     * Output: None
     *
     * Purpose: Render a legend of task‐colors and the per‐tick solves
     */
    void RecordTaskPanel()
    {
        tasksDoc.Clear();
        tasksDoc << "<div id='tasks-content'>";
        auto tasks = world.GetTasks();
        auto mons = world.GetSolveMonitors();

        for (size_t i = 0; i < tasks.size(); ++i)
        {
            std::string swatch_color = emp::ColorHSV(TaskHue(i), 1.0, 1.0);
            int solves = mons[i]->GetTotal();
            tasksDoc << "<div class='task-entry'>"
                     << "<span class='task-swatch' style='background:" << swatch_color << "'></span>"
                     << tasks[i]->name()
                     << " — Solves: " << solves
                     << "</div>";
        }

        tasksDoc << "</div>";
    }

    /**
     * Input: None
     *
     * Output: None
     *
     * Purpose: Render a list of cell-IDs being sent at any moment
     */
    void RecordCellPanel()
    {
        cellsDoc.Clear();
        cellsDoc << "<h4>Cell ID Sent / Received</h4>";

        auto sendMons = world.GetSendMonitors();
        auto recvMons = world.GetRecvMonitors();
        auto sendOtherMon = world.GetSendOtherMon(); 
        auto recvOtherMon = world.GetRecvOtherMon();
        const int total = num_w_boxes * num_h_boxes;

        for (int i = 0; i < total; i++)
        {
            int sends = sendMons[i]->GetTotal();
            int recvs = recvMons[i]->GetTotal();

            if (sends == 0 && recvs == 0) 
            continue;

            unsigned int cell_id = world.GetCellByLinearIndex(i)->GetID();
            cellsDoc << "<div class='cell-entry'>"
                    << "Cell " << cell_id
                    << " — Sent: " << sends
                    << ", Received: " << recvs
                    << "</div>";
           
        }
        int oSends = sendOtherMon->GetTotal();
        int oRecvs = recvOtherMon->GetTotal();
        if (oSends != 0 || oRecvs != 0) {
                cellsDoc << "<div class='non-cell-entry'>"
                        << "Non ID " << " value"
                        << " — Sent: " << oSends
                        << ", Received: " << oRecvs
                        << "</div>";
            }
    }

    /**
     * Input: A task id
     *
     * Output: A hue value for the task
     *
     * Purpose: Calculate the hue of the task based on its id and the global number of tasks.
     */
    double TaskHue(size_t task_id)
    {
        const size_t N = world.GetTasks().size();
        const size_t M = N * 2;
        const size_t slot = 2 * task_id + 1;
        // make color
        /**
         * |        Color | Hue (°) | `emp::ColorHSV(hue,1.0,1.0)`     |
           | -----------: | ------: | -------------------------------- |
           |          Red |       0 | `emp::ColorHSV(  0.0, 1.0, 1.0)` |
           |       Orange |      30 | `emp::ColorHSV( 30.0, 1.0, 1.0)` |
           |       Yellow |      60 | `emp::ColorHSV( 60.0, 1.0, 1.0)` |
           |   Chartreuse |      90 | `emp::ColorHSV( 90.0, 1.0, 1.0)` |
           |        Green |     120 | `emp::ColorHSV(120.0, 1.0, 1.0)` |
           | Spring Green |     150 | `emp::ColorHSV(150.0, 1.0, 1.0)` |
           |         Cyan |     180 | `emp::ColorHSV(180.0, 1.0, 1.0)` |
           |        Azure |     210 | `emp::ColorHSV(210.0, 1.0, 1.0)` |
           |         Blue |     240 | `emp::ColorHSV(240.0, 1.0, 1.0)` |
           |       Violet |     270 | `emp::ColorHSV(270.0, 1.0, 1.0)` |
           |      Magenta |     300 | `emp::ColorHSV(300.0, 1.0, 1.0)` |
           |         Rose |     330 | `emp::ColorHSV(330.0, 1.0, 1.0)` |

         */
        return 360.0 * double(slot) / double(M);
    }

    /**
     * Input: Current id an organism wwill send if it do so
     *
     * Output: A brightness value between 0 and 1
     *
     * Purpose: Calculate the brightness of the organism based on its message id relative to the max id known in the world.
     */
    double OrgBrightness(unsigned int cur_id)
    {
        const unsigned int id_min = min_known_id;
        const unsigned int id_max = max_known_id;

        const double bright_min = worldConfig.MIN_BRIGHT();
        const double bright_max = worldConfig.MAX_BRIGHT();

        if (id_max <= id_min)
            return bright_max;

        double ratio = (static_cast<double>(cur_id) - id_min) / static_cast<double>(id_max - id_min);

        ratio = std::clamp(ratio, 0.0, 1.0);

        return bright_min + ratio * (bright_max - bright_min);
    }

    /**
     * Input: An organism
     *
     * Output: A string describing the color the organism should be
     *
     * Purpose: Calculate the color of the organism based on its best task done.
     */
    std::string OrgColor(Organism &org)
    {
        // basic stats
        Cell *cur_cell = org.GetCell();
        unsigned int cur_id = cur_cell->GetID();
        unsigned int cur_message = org.GetMessage();
        // double pts = org.GetPoints();
        size_t best = org.GetBestTask();

        double brightness = OrgBrightness(cur_id);
        std::string fill;
        if ((cur_message && cur_message == max_known_id) || cur_id == max_known_id)
        {
            fill = emp::ColorHSV(0, 1.0, brightness);
        }
        else
        {
            fill = emp::ColorHSV(TaskHue(best), 1.0, 1.0);
        }
        return fill;
    }

    /**
     * Input: None
     *
     * Output: None
     *
     * Purpose: Load the global varibles keeping track of max and min known IDs, for when the full board is not filled.
     */
    void GetKnownIDRange()
    {
        int org_num = 0;
        for (int x = 0; x < num_w_boxes; x++)
        {
            for (int y = 0; y < num_h_boxes; y++)
            {
                if (world.IsOccupied(org_num))
                {
                    unsigned int new_id = world.GetCellByGridCoord(x, y)->GetID();
                    if (max_known_id)
                    {
                        max_known_id = std::max(max_known_id, new_id);
                    }
                    else
                    {
                        max_known_id = new_id;
                    }

                    if (min_known_id)
                    {
                        min_known_id = std::min(min_known_id, new_id);
                    }
                    else
                    {
                        min_known_id = new_id;
                    }
                }
                org_num++;
            }
        }
    }

    /**
     * Input: None
     *
     * Output: None
     *
     * Purpose: Update the frame showing the world and draw it again.and_eq
     */
    void DoFrame() override
    {
        canvas.Clear();
        world.Update();

        Stats st = ComputeStats();
        StatsRecord(st);
        RecordTaskPanel();
        RecordCellPanel();

        if (st.count < num_h_boxes * num_w_boxes)
        {
            GetKnownIDRange();
        }
        else
        {
            max_known_id = world.GetMaxID();
            min_known_id = world.GetMinID();
        }

        int org_num = 0;
        for (int x = 0; x < num_w_boxes; x++)
        {
            for (int y = 0; y < num_h_boxes; y++)
            {
                if (!world.IsOccupied(org_num))
                {
                    canvas.Rect(x * RECT_SIDE, y * RECT_SIDE, RECT_SIDE, RECT_SIDE,
                                "white", "black");
                }
                else
                {
                    Organism org = world.GetOrg(org_num);
                    Cell *cur_cell = world.GetCellByGridCoord(x, y);

                    std::string cell_color = OrgColor(org);

                    std::string isMax = "";
                    if (cur_cell->GetID() == max_known_id)
                    {
                        isMax = " MAX ";
                    }

                    std::string arrow_color = "black";
                    canvas.Rect(x * RECT_SIDE, y * RECT_SIDE, RECT_SIDE, RECT_SIDE,
                                cell_color, "black");

                    Cell *C = world.GetCellByLinearIndex(org_num);
                    int dir = C->GetFacing();
                    double cx = x * RECT_SIDE + (RECT_SIDE / 2.0);
                    double cy = y * RECT_SIDE + (RECT_SIDE / 2.0);

                    double step = 0.4 * RECT_SIDE;
                    double ex = cx + dir_dx[dir] * step;
                    double ey = cy + dir_dy[dir] * step;

                    canvas.Line(cx, cy, ex, ey, arrow_color, 2);
                }

                org_num++;
            }
        }
    
        std::cout << "Cycle: " << cycle << std::endl;
        cycle = cycle + 1;
    }
};

AEAnimator animator;
int main() { animator.Step(); }