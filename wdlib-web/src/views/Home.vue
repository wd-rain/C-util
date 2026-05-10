<template>
  <div class="home">
    <section class="hero">
      <h1>WDLib Blog</h1>
      <p>Embedded Development Notes & Sharing</p>
    </section>
    <section class="post-list">
      <PostCard v-for="post in posts" :key="post.id" :post="post" />
    </section>
  </div>
</template>

<script setup>
import PostCard from '../components/PostCard.vue'
import { posts } from '../posts'
</script>

<style scoped>
.hero {
  text-align: center;
  padding: 3rem 0 2rem;
}

.hero h1 {
  font-size: 2.2rem;
  color: #1a1a2e;
  margin-bottom: 0.4rem;
}

.hero p {
  color: #666;
  font-size: 1.05rem;
}

.post-list {
  max-width: 720px;
  margin: 0 auto;
}
</style>
