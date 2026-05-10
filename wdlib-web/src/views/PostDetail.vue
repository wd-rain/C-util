<template>
  <div class="post-detail" v-if="post">
    <article class="article">
      <h1>{{ post.title }}</h1>
      <div class="meta">
        <span class="date">{{ post.date }}</span>
        <span class="tag" v-for="tag in post.tags" :key="tag">{{ tag }}</span>
      </div>
      <div class="content" v-html="post.content"></div>
    </article>
    <router-link to="/" class="back">&larr; Back to home</router-link>
  </div>
  <div v-else class="not-found">
    <h2>Post not found</h2>
    <router-link to="/">Back to home</router-link>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useRoute } from 'vue-router'
import { posts } from '../posts'

const route = useRoute()
const post = computed(() => posts.find(p => p.id === route.params.id))
</script>

<style scoped>
.post-detail {
  max-width: 720px;
  margin: 0 auto;
  padding: 2rem 0;
}

.article h1 {
  font-size: 1.8rem;
  color: #1a1a2e;
  margin-bottom: 0.8rem;
}

.meta {
  display: flex;
  align-items: center;
  gap: 0.6rem;
  margin-bottom: 2rem;
  padding-bottom: 1rem;
  border-bottom: 1px solid #eee;
}

.date {
  color: #888;
  font-size: 0.9rem;
}

.tag {
  background: #e8f4f8;
  color: #0a7ea4;
  padding: 0.15rem 0.5rem;
  border-radius: 3px;
  font-size: 0.8rem;
}

.content {
  line-height: 1.8;
  color: #333;
}

.content :deep(h3) {
  margin: 1.5rem 0 0.8rem;
  color: #1a1a2e;
}

.content :deep(ul) {
  padding-left: 1.5rem;
  margin-bottom: 1rem;
}

.content :deep(li) {
  margin-bottom: 0.4rem;
}

.content :deep(p) {
  margin-bottom: 1rem;
}

.back {
  display: inline-block;
  margin-top: 2rem;
  color: #0a7ea4;
  text-decoration: none;
}

.back:hover {
  text-decoration: underline;
}

.not-found {
  text-align: center;
  padding: 4rem 0;
}

.not-found a {
  color: #0a7ea4;
}
</style>
