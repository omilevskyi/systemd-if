#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <systemd/sd-bus.h>

#ifndef VERSION
#define VERSION "devel"
#endif

#define START "start"
#define STOP  "stop"

static int verbose = 0;

static const char *marker_file = NULL;

static const unsigned pre_stop_delay_ms = 2000;
static const unsigned poll_interval_ms = 500;
static const int max_poll_attempts = 5;

static int usage(const char *prog, int rc) {
  fprintf(stderr,
          "Usage: %s [-f FILE|--file FILE] [-h|--help] [-v|--verbose]"
          " [-V|--version] <" START "|" STOP "> <unit>\n",
          prog);
  return rc;
}

static int version(int rc) {
  fprintf(stdout, "%s\n", VERSION);
  return rc;
}

static void sleep_ms(unsigned ms) { usleep((useconds_t)(ms * 1000)); }

static void verb_event(const char *prog_name, const char *action) {
  if (verbose) {
    const char *file_name;

    file_name = strrchr(marker_file, '/');
    file_name = file_name ? file_name + 1 : marker_file;

    fprintf(stderr, "%s %s: %s is %s\n", prog_name, action, file_name,
            access(marker_file, F_OK) == 0 ? "present" : "missing");
  }
}

static int systemd_unit_action(const char *action, const char *unit) {
  sd_bus *bus = NULL;
  sd_bus_error error = SD_BUS_ERROR_NULL;
  sd_bus_message *reply = NULL;

  int rc;
  const char *method;

  if (strcmp(action, START) == 0) {
    method = "StartUnit";
  } else if (strcmp(action, STOP) == 0) {
    method = "StopUnit";
  } else {
    return -EINVAL;
  }

  rc = sd_bus_open_system(&bus);
  if (rc < 0) {
    fprintf(stderr, "sd_bus_open_system(): %s\n", strerror(-rc));
    goto cleanup;
  }

  rc = sd_bus_call_method(bus, "org.freedesktop.systemd1",
                          "/org/freedesktop/systemd1",
                          "org.freedesktop.systemd1.Manager", method, &error,
                          &reply, "ss", unit, "replace");

  if (rc < 0) {
    fprintf(stderr, "%s(%s) failed: %s%s%s\n", method, unit,
            error.name ? error.name : "", error.name ? ": " : "",
            error.message ? error.message : strerror(-rc));
    goto cleanup;
  }

  rc = 0;

cleanup:
  sd_bus_error_free(&error);
  sd_bus_message_unref(reply);
  sd_bus_unref(bus);

  return rc;
}

int main(int argc, char *argv[]) {
  int opt;
  int i = 0;
  const char *action;
  const char *unit;
  const char *prog_name;
  static const struct option long_options[] = {
      {"file", required_argument, NULL, 'f'},
      {"help", no_argument, NULL, 'h'},
      {"verbose", no_argument, NULL, 'v'},
      {"version", no_argument, NULL, 'V'},
      {NULL, 0, NULL, 0},
  };

  prog_name = basename(argv[0]);

  while ((opt = getopt_long(argc, argv, "f:hvV", long_options, NULL)) != -1) {
    switch (opt) {
    case 'f':
      marker_file = optarg;
      break;
    case 'v':
      verbose = 1;
      break;
    case 'h':
      return usage(prog_name, EXIT_SUCCESS);
    case 'V':
      return version(EXIT_SUCCESS);
    default:
      return usage(prog_name, 201);
    }
  }

  if (marker_file == NULL) {
    fprintf(stderr, "Marker file is not specified\n");
    return usage(prog_name, 202);
  }

  if ((argc - optind) != 2)
    return usage(prog_name, 200);

  action = argv[optind];
  unit = argv[optind + 1];

  verb_event(prog_name, action);

  if (strcmp(action, START) == 0) {
    while (i < max_poll_attempts) {
      if (access(marker_file, F_OK) == 0)
        break;
      ++i;
      sleep_ms(poll_interval_ms);
    }
  } else if (strcmp(action, STOP) == 0) {
    sleep_ms(pre_stop_delay_ms);
    verb_event(prog_name, action);
    while (i < max_poll_attempts) {
      if (access(marker_file, F_OK) != 0)
        break;
      ++i;
      sleep_ms(poll_interval_ms);
    }
  } else {
    return usage(prog_name, 203);
  }

  if (verbose)
    fprintf(stderr, "%s %s: i=%d\n", prog_name, action, i);

  verb_event(prog_name, action);
  if (i >= max_poll_attempts) {
    fprintf(stderr, "%s %s: no action\n", prog_name, action);
    if (verbose)
      fprintf(stderr, "%s %s: return false (rc=1)\n", prog_name, action);
    return 1;
  }

  if (verbose)
    fprintf(stderr, "%s %s: return true (rc=0)\n", prog_name, action);

  int rc = systemd_unit_action(action, unit);
  if (rc < 0)
    return -rc;

  return 0;
}
