#ifndef MOCKS_MOCK_NODE_MANAGER_H
#define MOCKS_MOCK_NODE_MANAGER_H

#include "fvm/interfaces/INodeManager.h"
#include "fvm/interfaces/ISystemClock.h"
#include <map>
#include <string>
#include <vector>

namespace fvm {
namespace mocks {

/**
 * @brief Mock implementation of INodeManager for testing
 *
 * Provides in-memory storage of node metadata for isolated testing.
 * Allows direct manipulation of node data without file I/O.
 */
class MockNodeManager : public interfaces::INodeManager {
private:
    struct NodeData {
        unsigned long long id;
        std::string name;
        std::string content;
        std::string create_time;
        std::string update_time;
        unsigned long long counter;
    };

    std::map<unsigned long long, NodeData> nodes_;
    unsigned long long next_id_ = 1;
    interfaces::ISystemClock* clock_ = nullptr;
    bool initialized_ = false;

    // Helper to get current time
    std::string get_current_time() const {
        if (clock_) {
            return clock_->get_current_time(8);  // Default timezone
        }
        return "2026-01-22 00:00:00";
    }

public:
    MockNodeManager() = default;
    virtual ~MockNodeManager() = default;

    // System clock injection
    void set_system_clock(interfaces::ISystemClock* clock) override {
        clock_ = clock;
    }

    // Lifecycle management
    bool initialize() override {
        initialized_ = true;
        return true;
    }

    bool shutdown() override {
        return true;
    }

    // Node existence check
    bool node_exist(unsigned long long id) override {
        return nodes_.find(id) != nodes_.end();
    }

    // Create new node
    unsigned long long get_new_node(const std::string& name) override {
        unsigned long long id = next_id_++;
        std::string time = get_current_time();
        nodes_[id] = NodeData{id, name, "", time, time, 0};
        return id;
    }

    // Delete node
    void delete_node(unsigned long long idx) override {
        nodes_.erase(idx);
    }

    // Update content
    unsigned long long update_content(unsigned long long idx, const std::string& content) override {
        auto it = nodes_.find(idx);
        if (it != nodes_.end()) {
            it->second.content = content;
            it->second.update_time = get_current_time();
            return idx;
        }
        return 0;
    }

    // Update name
    unsigned long long update_name(unsigned long long idx, const std::string& name) override {
        auto it = nodes_.find(idx);
        if (it != nodes_.end()) {
            it->second.name = name;
            it->second.update_time = get_current_time();
            return idx;
        }
        return 0;
    }

    // Get content
    std::string get_content(unsigned long long idx) override {
        auto it = nodes_.find(idx);
        if (it != nodes_.end()) {
            return it->second.content;
        }
        return "";
    }

    // Get name
    std::string get_name(unsigned long long idx) override {
        auto it = nodes_.find(idx);
        if (it != nodes_.end()) {
            return it->second.name;
        }
        return "";
    }

    // Get update time
    std::string get_update_time(unsigned long long idx) override {
        auto it = nodes_.find(idx);
        if (it != nodes_.end()) {
            return it->second.update_time;
        }
        return "";
    }

    // Get create time
    std::string get_create_time(unsigned long long idx) override {
        auto it = nodes_.find(idx);
        if (it != nodes_.end()) {
            return it->second.create_time;
        }
        return "";
    }

    // Increase counter
    void increase_counter(unsigned long long idx) override {
        auto it = nodes_.find(idx);
        if (it != nodes_.end()) {
            it->second.counter++;
        }
    }

    // Get counter
    unsigned long long _get_counter(unsigned long long idx) override {
        auto it = nodes_.find(idx);
        if (it != nodes_.end()) {
            return it->second.counter;
        }
        return 0;
    }

    // ===== Test helper methods =====

    // Clear all nodes
    void clear() {
        nodes_.clear();
        next_id_ = 1;
    }

    // Get number of nodes
    size_t size() const {
        return nodes_.size();
    }

    // Check if initialized
    bool is_initialized() const {
        return initialized_;
    }

    // Directly add a node with specific data (for testing)
    void add_node(unsigned long long id, const std::string& name, const std::string& content = "") {
        std::string time = get_current_time();
        nodes_[id] = NodeData{id, name, content, time, time, 0};
        if (id >= next_id_) {
            next_id_ = id + 1;
        }
    }

    // Set next ID to use (for testing)
    void set_next_id(unsigned long long id) {
        next_id_ = id;
    }
};

} // namespace mocks
} // namespace fvm

#endif // MOCKS_MOCK_NODE_MANAGER_H
