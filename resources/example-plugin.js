// FX16 plugin example.
// Every *.js file in this folder is loaded at startup as an ES module.
// Files whose names start with "_" are ignored. Edit or delete this file freely.
// Docs: README.md, section "Plugins".
import * as std from "std";

fx16.menu.add({
  title: "Hello plugin",
  description: "example menu item written in JavaScript",
  tag: "js",
  onSelect() {
    const ru = fx16.lang === "ru";
    const choice = fx16.ui.pick("Hello", [
      { title: ru ? "Сказать привет" : "Say hello", description: ru ? "простое сообщение" : "a simple message" },
      { title: ru ? "Установленные программы" : "Installed programs", description: ru ? "список из FX16" : "list from FX16" },
      { title: ru ? "Спросить имя" : "Ask my name", description: ru ? "ввод строки" : "text input" },
    ]);
    if (choice === 0) {
      const user = std.getenv("USERNAME") || "friend";
      fx16.ui.message((ru ? "Привет из JavaScript, " : "Hello from JavaScript, ") + user + "!", "ok");
    } else if (choice === 1) {
      const list = fx16.installed().map((p) => p.name + " " + p.version);
      fx16.ui.message(list.length ? list.join("\n") : (ru ? "Пока ничего не установлено." : "Nothing installed yet."));
    } else if (choice === 2) {
      const name = fx16.ui.ask(ru ? "Как вас зовут?" : "What is your name?", "friend");
      fx16.ui.message((ru ? "Приятно познакомиться, " : "Nice to meet you, ") + name + "!", "ok");
    }
  },
});
