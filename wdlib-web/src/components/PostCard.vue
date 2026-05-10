<template>
  <article class="post-card">
    <router-link :to="`/post/${post.id}`" class="post-title">
      <h2>{{ post.title }}</h2>
    </router-link>
    <div class="post-meta">
      <span class="date">{{ post.date }}</span>
      <span class="tag" v-for="tag in post.tags" :key="tag">{{ tag }}</span>
    </div>
    <p class="post-excerpt">{{ post.excerpt }}</p>
    <router-link :to="`/post/${post.id}`" class="read-more">Read more &rarr;</router-link>
  </article>
</template>

<script setup>
defineProps({
  post: Object
})
</script>

<style scoped>
.post-card {
  background: #fff;
  border-radius: 8px;
  padding: 1.5rem 2rem;
  margin-bottom: 1.5rem;
  box-shadow: 0 1px 4px rgba(0, 0, 0, 0.06);
  transition: box-shadow 0.2s;
}

.post-card:hover {
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
}

.post-title {
  text-decoration: none;
  color: #1a1a2e;
}

.post-title h2 {
  font-size: 1.3rem;
  margin-bottom: 0.5rem;
}

.post-title:hover h2 {
  color: #00d2ff;
}

.post-meta {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  margin-bottom: 0.8rem;
  font-size: 0.85rem;
}

.date {
  color: #888;
}

.tag {
  background: #e8f4f8;
  color: #0a7ea4;
  padding: 0.15rem 0.5rem;
  border-radius: 3px;
  font-size: 0.8rem;
}

.post-excerpt {
  color: #555;
  line-height: 1.7;
  margin-bottom: 0.8rem;
}

.read-more {
  color: #0a7ea4;
  text-decoration: none;
  font-size: 0.9rem;
}

.read-more:hover {
  text-decoration: underline;
}
</style>
