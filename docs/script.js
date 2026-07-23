document.documentElement.classList.add("js");

const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)");
const finePointer = window.matchMedia("(pointer: fine)");
const navToggle = document.querySelector(".nav-toggle");
const nav = document.querySelector("#site-nav");
const progress = document.querySelector(".scroll-progress span");
const video = document.querySelector(".video-shell video");
const videoToggle = document.querySelector(".video-toggle");
let lastFocused = null;
let ticking = false;

const closeNav = () => {
  document.body.classList.remove("nav-open");
  navToggle?.setAttribute("aria-expanded", "false");
  navToggle?.querySelector(".sr-only")?.replaceChildren("開啟導覽選單");
};

navToggle?.addEventListener("click", () => {
  const willOpen = !document.body.classList.contains("nav-open");
  lastFocused = willOpen ? document.activeElement : lastFocused;
  document.body.classList.toggle("nav-open", willOpen);
  navToggle.setAttribute("aria-expanded", String(willOpen));
  navToggle.querySelector(".sr-only")?.replaceChildren(willOpen ? "關閉導覽選單" : "開啟導覽選單");
  if (willOpen) nav?.querySelector("a")?.focus();
});

nav?.addEventListener("click", (event) => {
  if (event.target.closest("a")) closeNav();
});

document.addEventListener("keydown", (event) => {
  if (event.key === "Escape" && document.body.classList.contains("nav-open")) {
    closeNav();
    lastFocused?.focus();
  }
});

const updateScroll = () => {
  const max = document.documentElement.scrollHeight - window.innerHeight;
  const ratio = max > 0 ? window.scrollY / max : 0;
  if (progress) progress.style.transform = `scaleX(${ratio})`;
  ticking = false;
};

window.addEventListener("scroll", () => {
  if (!ticking) {
    ticking = true;
    window.requestAnimationFrame(updateScroll);
  }
}, { passive: true });
updateScroll();

const revealItems = document.querySelectorAll("[data-reveal]");
revealItems.forEach((item) => {
  item.style.setProperty("--reveal-delay", `${item.dataset.revealDelay || 0}ms`);
});

const activateStatic = () => {
  revealItems.forEach((item) => item.classList.add("is-visible"));
  document.querySelectorAll("[data-flow], [data-pipeline]").forEach((item) => item.classList.add("is-active"));
};

if (!("IntersectionObserver" in window) || reducedMotion.matches) {
  activateStatic();
} else {
  const revealObserver = new IntersectionObserver((entries, observer) => {
    entries.forEach((entry) => {
      if (entry.isIntersecting) {
        entry.target.classList.add("is-visible");
        observer.unobserve(entry.target);
      }
    });
  }, { threshold: 0.13 });
  revealItems.forEach((item) => revealObserver.observe(item));

  const flowObserver = new IntersectionObserver((entries) => {
    entries.forEach((entry) => entry.target.classList.toggle("is-active", entry.isIntersecting));
  }, { threshold: 0.35 });
  document.querySelectorAll("[data-flow], [data-pipeline]").forEach((item) => flowObserver.observe(item));

  if (video) {
    const videoObserver = new IntersectionObserver((entries) => {
      entries.forEach((entry) => {
        if (entry.isIntersecting && !reducedMotion.matches) {
          video.play().catch(() => {});
        } else {
          video.pause();
        }
        videoToggle?.replaceChildren(video.paused ? "播放預覽" : "暫停預覽");
      });
    }, { threshold: 0.55 });
    videoObserver.observe(video);
  }
}

videoToggle?.addEventListener("click", () => {
  if (video.paused) {
    video.play().catch(() => {});
  } else {
    video.pause();
  }
  videoToggle.replaceChildren(video.paused ? "播放預覽" : "暫停預覽");
});
video?.addEventListener("play", () => videoToggle?.replaceChildren("暫停預覽"));
video?.addEventListener("pause", () => videoToggle?.replaceChildren("播放預覽"));

if (finePointer.matches && !reducedMotion.matches) {
  document.querySelectorAll("[data-tilt]").forEach((card) => {
    card.addEventListener("pointermove", (event) => {
      const rect = card.getBoundingClientRect();
      const rotateX = ((event.clientY - rect.top) / rect.height - 0.5) * -3;
      const rotateY = ((event.clientX - rect.left) / rect.width - 0.5) * 3;
      card.style.transform = `perspective(900px) rotateX(${rotateX}deg) rotateY(${rotateY}deg)`;
    }, { passive: true });
    card.addEventListener("pointerleave", () => {
      card.style.transform = "";
    }, { passive: true });
  });
}

const sections = [...document.querySelectorAll("main section[id]")];
const navLinks = [...document.querySelectorAll("#site-nav a[href^='#']")];
if ("IntersectionObserver" in window) {
  const spyObserver = new IntersectionObserver((entries) => {
    entries.forEach((entry) => {
      if (!entry.isIntersecting) return;
      navLinks.forEach((link) => {
        const active = link.getAttribute("href") === `#${entry.target.id}`;
        link.classList.toggle("active", active);
        if (active) link.setAttribute("aria-current", "location");
        else link.removeAttribute("aria-current");
      });
    });
  }, { rootMargin: "-35% 0px -58%", threshold: 0 });
  sections.forEach((section) => spyObserver.observe(section));
}

document.addEventListener("visibilitychange", () => {
  document.body.classList.toggle("is-paused", document.hidden);
  if (document.hidden) video?.pause();
});
