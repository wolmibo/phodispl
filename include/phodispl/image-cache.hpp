#ifndef PHODISPL_IMAGE_CACHE_HPP_INCLUDED
#define PHODISPL_IMAGE_CACHE_HPP_INCLUDED

#include "phodispl/image.hpp"

#include <filesystem>
#include <memory>
#include <vector>



class image_cache {
  public:
    image_cache(
        std::move_only_function<void(const std::shared_ptr<image>&, size_t) const> load,
        std::move_only_function<void(const std::shared_ptr<image>&, bool)   const> unload
    ) :
      load_function_  {std::move(load)},
      unload_function_{std::move(unload)}
    {}



    void set(std::span<const std::filesystem::path>);

    void add       (const std::filesystem::path&);
    void remove    (const std::filesystem::path&);
    void invalidate(const std::filesystem::path&);

    void invalidate(size_t);

    void invalidate_current();
    void invalidate_all();

    void seek(ssize_t);
    void next()        { seek( 1); }
    void previous()    { seek(-1); }

    void ensure_loaded() const;


    [[nodiscard]] std::shared_ptr<image> current() const;



    [[nodiscard]] bool   empty() const { return images_.empty(); }
    [[nodiscard]] size_t size()  const { return images_.size(); }





  private:
    std::move_only_function<void(const std::shared_ptr<image>&, size_t) const>
      load_function_;
    std::move_only_function<void(const std::shared_ptr<image>&, bool) const>
      unload_function_;

    std::vector<std::shared_ptr<image>> images_;
    size_t                              index_{0};



    void load_maybe(size_t) const;

    void load_unsafe(size_t, size_t) const;
    void unload_unsafe(size_t) const;



    void cleanup(size_t) const;
};

#endif // PHODISPL_IMAGE_CACHE_HPP_INCLUDED
