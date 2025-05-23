#define UIT_VENDORIZE_EMP
#define UIT_SUPPRESS_MACRO_INSEEP_WARNINGS

#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/Document.hpp"
#include "emp/web/web.hpp"
#include "World.h"
#include "Org.h"
#include "ConfigSetup.h"
#include "emp/config/ArgManager.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/web/UrlParams.hpp"

emp::web::Document doc("target");
emp::web::Document settings("settings");
emp::web::Document stats("stats");
emp::web::Document tasksDoc("tasks");
// Your configuration object needs to exist outside of the animator class
MyConfigType worldConfig;

struct Stats {
    int count;
    double min;
    double max;
    double mean;
    double variance;
};

class AEAnimator : public emp::web::Animate {

    // arena width and height
    const int num_h_boxes = worldConfig.WORLD_LEN();
    const int num_w_boxes = worldConfig.WORLD_WIDTH();
    const double RECT_SIDE = worldConfig.CELL_SIZE();
    const double width{num_w_boxes * RECT_SIDE};
    const double height{num_h_boxes * RECT_SIDE};
    emp::Random random{worldConfig.SEED()};
    OrgWorld world{random};

    emp::web::Canvas canvas{width, height, "canvas"};

public:
    // Constructor
    AEAnimator() {
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
    void SetupCanvas() {
        // shove canvas into the div
        // along with a control button
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
    void SetupConfigPanel() {
        // setup configuration panel
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
        // apply configuration query params and config files to config
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
     * Purpose: Setup the world and the initial organisms, then add it to the document.
     */
    void SetupWorld() {
        // setup the world
        world.SetPopStruct_Grid(num_w_boxes, num_h_boxes);
        world.Resize(num_h_boxes, num_w_boxes);
        for (int i = 0; i < worldConfig.START_NUM(); i++) {
            Organism *new_org = new Organism(&world);
            world.Inject(*new_org);
        }
    }

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

        for (int i = 0; i < world.GetSize(); ++i) {
            if (!world.IsOccupied(i))
                continue;
            double p = world.GetOrg(i).GetPoints();
            s.max = std::max(s.max, p);
            s.min = std::min(s.min, p);
            sum += p;
            sum_sq += p * p;
            ++s.count;
        }
        if (s.count == 0) {
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
    void RecordTaskPanel() {
        tasksDoc.Clear();
        tasksDoc << "<div id='tasks-content'>";
        auto tasks = world.GetTasks();
        auto mons  = world.GetSolveMonitors();
      
        for (size_t i = 0; i < tasks.size(); ++i) {
          std::string swatch_color = emp::ColorHSV(TaskHue(i), 1.0, 1.0);
          int solves = mons[i]->GetTotal();
          tasksDoc << "<div class='task-entry'>"
                   <<   "<span class='task-swatch' style='background:" << swatch_color << "'></span>"
                   <<   tasks[i]->name()
                   <<   " — Solves: " << solves
                   << "</div>";
        }
      
        tasksDoc << "</div>";
      }
      

    /**
     * Input: A task id
     *
     * Output: A hue value for the task
     *
     * Purpose: Calculate the hue of the task based on its id and the global number of tasks.
     */
    double TaskHue(size_t task_id) {
        size_t N = world.GetTasks().size();
        return 360.0 * double(task_id) / (N+1);
    }

    /**
     * Input: A point value and the max points in the world
     *
     * Output: A brightness value between 0 and 1
     *
     * Purpose: Calculate the brightness of the organism based on its points relative to the max point in the world.
     */
    double PtsBrightness(double pts, double max_pts) {
        double minB = worldConfig.MIN_BRIGHT();
        double maxB = worldConfig.MAX_BRIGHT();
        double ratio_pts = (max_pts > 0)
                               ? std::clamp(pts / max_pts, 0.0, 1.0)
                               : 0.0;
        double brightness = minB + ratio_pts * (maxB - minB);
        return std::clamp(brightness, 0.0, 1.0);
    }

    /**
     * Input: An organism and the current max points in the world
     *
     * Output: A string describing the color the organism should be
     *
     * Purpose: Calculate the color of the organism based on its best task done and points.
     */
    std::string OrgColor(Organism &org, float max_pts) {
        // basic stats
        double pts = org.GetPoints();
        size_t best = org.GetBestTask();

        // make color
        std::string fill = emp::ColorHSV(TaskHue(best), 1.0, PtsBrightness(pts, max_pts));
        return fill;
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

        int org_num = 0;
        for (int x = 0; x < num_w_boxes; x++) {
            for (int y = 0; y < num_h_boxes; y++){
                if (!world.IsOccupied(org_num)) {
                    canvas.Rect(x * RECT_SIDE, y * RECT_SIDE, RECT_SIDE, RECT_SIDE,
                                "white", "black");
                }
                else {
                    auto &org = world.GetOrg(org_num);
                    const std::string fill = OrgColor(org, st.max);
                    canvas.Rect(x * RECT_SIDE, y * RECT_SIDE, RECT_SIDE, RECT_SIDE,
                                fill, "black");
                }
                org_num++;
            }
        }
    }
};

AEAnimator animator;
int main() { animator.Step(); }