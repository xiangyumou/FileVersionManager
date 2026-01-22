#ifndef MOCKS_MOCK_REPOSITORY_H
#define MOCKS_MOCK_REPOSITORY_H

#include "fvm/repositories/IFileManagerRepository.h"
#include "fvm/repositories/INodeManagerRepository.h"
#include "fvm/repositories/IVersionManagerRepository.h"
#include "fvm/repositories/ICommandRepository.h"
#include <map>
#include <string>
#include <vector>

// Forward declarations
struct fileNode;
struct commandNode;

namespace fvm {
namespace mocks {

// ===== Mock FileManager Repository =====
class MockFileManagerRepository : public repositories::IFileManagerRepository {
private:
    std::map<unsigned long long, fileNode> storage_;
    bool fail_on_save_ = false;
    bool fail_on_load_ = false;

public:
    bool save(const std::map<unsigned long long, fileNode>& data) override {
        if (fail_on_save_) return false;
        storage_ = data;
        return true;
    }

    bool load(std::map<unsigned long long, fileNode>& data) override {
        if (fail_on_load_) return false;
        data = storage_;
        return true;
    }

    // Test control methods
    void set_save_failure(bool fail) { fail_on_save_ = fail; }
    void set_load_failure(bool fail) { fail_on_load_ = fail; }
    void clear() { storage_.clear(); }
    size_t size() const { return storage_.size(); }
};

// ===== Mock Node Manager Repository =====
struct NodeData {
    std::string name;
    std::string create_time;
    std::string update_time;
    unsigned long long fid;
    unsigned long long cnt;
};

class MockNodeManagerRepository : public repositories::INodeManagerRepository {
private:
    std::map<unsigned long long, NodeData> storage_;
    bool fail_on_save_ = false;
    bool fail_on_load_ = false;

public:
    bool save(const std::map<unsigned long long, NodeData>& data) override {
        if (fail_on_save_) return false;
        storage_ = data;
        return true;
    }

    bool load(std::map<unsigned long long, NodeData>& data) override {
        if (fail_on_load_) return false;
        data = storage_;
        return true;
    }

    // Test control methods
    void set_save_failure(bool fail) { fail_on_save_ = fail; }
    void set_load_failure(bool fail) { fail_on_load_ = fail; }
    void clear() { storage_.clear(); }
    size_t size() const { return storage_.size(); }

    // Pre-populate with test data
    void set_node(unsigned long long id, const NodeData& data) {
        storage_[id] = data;
    }
};

// ===== Mock Version Manager Repository =====
struct VersionNode {
    unsigned long long node_id;
    std::string create_time;
    std::string description;
    unsigned long long fid;
};

class MockVersionManagerRepository : public repositories::IVersionManagerRepository {
private:
    std::map<std::pair<unsigned long long, unsigned long long>, VersionNode> storage_;
    bool fail_on_save_ = false;
    bool fail_on_load_ = false;

public:
    bool save(const std::map<std::pair<unsigned long long, unsigned long long>, VersionNode>& data) override {
        if (fail_on_save_) return false;
        storage_ = data;
        return true;
    }

    bool load(std::map<std::pair<unsigned long long, unsigned long long>, VersionNode>& data) override {
        if (fail_on_load_) return false;
        data = storage_;
        return true;
    }

    // Test control methods
    void set_save_failure(bool fail) { fail_on_save_ = fail; }
    void set_load_failure(bool fail) { fail_on_load_ = fail; }
    void clear() { storage_.clear(); }
    size_t size() const { return storage_.size(); }
};

// ===== Mock Command Repository =====
class MockCommandRepository : public repositories::ICommandRepository {
private:
    std::map<unsigned long long, commandNode> storage_;
    bool fail_on_save_ = false;
    bool fail_on_load_ = false;

public:
    bool save(const std::map<unsigned long long, commandNode>& data) override {
        if (fail_on_save_) return false;
        storage_ = data;
        return true;
    }

    bool load(std::map<unsigned long long, commandNode>& data) override {
        if (fail_on_load_) return false;
        data = storage_;
        return true;
    }

    // Test control methods
    void set_save_failure(bool fail) { fail_on_save_ = fail; }
    void set_load_failure(bool fail) { fail_on_load_ = fail; }
    void clear() { storage_.clear(); }
    size_t size() const { return storage_.size(); }
};

} // namespace mocks
} // namespace fvm

#endif // MOCKS_MOCK_REPOSITORY_H
