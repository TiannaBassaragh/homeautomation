// Composables
import { createRouter, createWebHistory } from "vue-router";

const routes = [
  //home
  {
    path: "/",
    name: "Home",
    component: () => import("@/views/Home.vue"),
    meta: { transition: "fade" },
  },
  //control page
  {
    path: "/control",
    name: "Control",
    component: () => import("@/views/Control.vue"),
    meta: { transition: "fade" },
  },
  //dashboard page
  {
    path: "/dashboard",
    name: "Dashboard",
    component: () => import("@/views/Dashboard.vue"),
    meta: { transition: "fade" },
  },
  //analysis page
  {
    path: "/analysis",
    name: "Analysis",
    component: () => import("@/views/Analysis.vue"),
    meta: { transition: "fade" },
  },
];

const router = createRouter({
  history: createWebHistory(process.env.BASE_URL),
  routes,
});

export default router;
